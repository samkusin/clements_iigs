#include "clem_backend.hpp"
#include "emulator.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <optional>

#include "fmt/format.h"

static constexpr unsigned kSlabMemorySize = 32 * 1024 * 1024;

struct ClemensRunSampler {
  //  our method of keeping the simulation in sync with real time is to rely
  //  on two counters
  //    - sDT = simulation counter that ticks at a fixed rate, and
  //    - aDT = real-time timer that counts the actual time passed
  //
  //  if aDT < sDT, then sleep the amount of time needed to catch up with sDT
  //  if aDT >= sDT, then clamp aDT to sDT
  std::chrono::microseconds fixedTimeInterval;
  std::chrono::microseconds actualTimeInterval;
  std::chrono::microseconds sampledFrameTime;
  uint64_t seqNo;
  unsigned sampledFrameCount;
  double sampledFramesPerSecond;

  ClemensRunSampler() {
    reset();
    seqNo = 0;
  }

  void reset() {
    fixedTimeInterval = std::chrono::microseconds::zero();
    actualTimeInterval = std::chrono::microseconds::zero();
    sampledFrameTime = std::chrono::microseconds::zero();
    sampledFrameCount = 0;
    sampledFramesPerSecond = 0.0f;
  }

  void update(std::chrono::microseconds fixedFrameInterval,
              std::chrono::microseconds actualFrameInterval) {
    fixedTimeInterval += fixedFrameInterval;
    actualTimeInterval += actualFrameInterval;
    if (actualTimeInterval < fixedTimeInterval) {
      std::this_thread::sleep_for(fixedTimeInterval - actualTimeInterval);
      fixedTimeInterval -= actualTimeInterval;
      actualTimeInterval = std::chrono::microseconds::zero();
    } else {
      std::this_thread::yield();
      if (actualTimeInterval - fixedFrameInterval > std::chrono::microseconds(500000)) {
        actualTimeInterval = fixedTimeInterval = std::chrono::microseconds::zero();
      }
    }

    sampledFrameTime += actualFrameInterval;
    ++sampledFrameCount;

    if (sampledFrameTime >= std::chrono::microseconds(500000)) {
      sampledFramesPerSecond = sampledFrameCount * 1e6 / sampledFrameTime.count();
      sampledFrameTime = std::chrono::microseconds::zero();
      sampledFrameCount = 0;
    }
    ++seqNo;
  }
};

ClemensBackend::ClemensBackend(std::string romPathname, const Config& config,
                               PublishStateDelegate publishDelegate) :
  config_(config),
  slabMemory_(kSlabMemorySize, malloc(kSlabMemorySize)),
  emulatorStepsSinceReset_(0) {
  romBuffer_ = loadROM(romPathname.c_str());

  memset(&machine_, 0, sizeof(machine_));
  clemens_host_setup(&machine_, &ClemensBackend::emulatorLog, this);

  switch (config_.type) {
    case Type::Apple2GS:
      initApple2GS();
      break;
  }

  //  TODO: Only use this when opcode debugging is enabled
  clemens_opcode_callback(&machine_, &ClemensBackend::emulatorOpcodeCallback);

  //  Everything is ready for the main thread
  runner_ = std::thread(&ClemensBackend::main, this, std::move(publishDelegate));
}

ClemensBackend::~ClemensBackend() {
  terminate();
  runner_.join();

  free(slabMemory_.getHead());
}

void ClemensBackend::terminate() {
  queueToFront(Command{Command::Terminate});
}

void ClemensBackend::reset() {
  queue(Command{Command::ResetMachine});
}

void ClemensBackend::setRefreshFrequency(unsigned hz) {
  queue(Command{Command::SetHostUpdateFrequency, fmt::format("{}", hz)});
}

void ClemensBackend::run() {
  queue(Command{Command::RunMachine});
}

void ClemensBackend::publish() {
  queue(Command{Command::Publish});
}

void ClemensBackend::queue(const Command& cmd) {
  {
    std::lock_guard<std::mutex> queuelock(commandQueueMutex_);
    commandQueue_.emplace_back(cmd);
  }
  commandQueueCondition_.notify_one();
}

void ClemensBackend::queueToFront(const Command& cmd) {
  {
    std::lock_guard<std::mutex> queuelock(commandQueueMutex_);
    commandQueue_.emplace_front(cmd);
  }
  commandQueueCondition_.notify_one();
}

static int64_t calculateClocksPerTimeslice(ClemensMachine* machine, unsigned hz) {
  bool is_machine_slow;
  return int64_t(clemens_clocks_per_second(machine, &is_machine_slow) / hz);
}

void ClemensBackend::main(PublishStateDelegate publishDelegate) {
  int64_t clocksRemainingInTimeslice = 0;
  std::optional<int> stepsRemaining = 0;
  bool isActive = true;
  bool isMachineReady = false;

  ClemensRunSampler runSampler;

  unsigned emulatorRefreshFrequency = 60;
  auto fixedFrameInterval =
    std::chrono::microseconds((long)std::floor(1e6/emulatorRefreshFrequency));
  auto lastFrameTimePoint = std::chrono::high_resolution_clock::now();
  while (isActive) {
    bool isRunning = !stepsRemaining.has_value() || *stepsRemaining > 0;
    bool publishState = false;

    std::unique_lock<std::mutex> queuelock(commandQueueMutex_);
    if (!isRunning) {
      //  waiting for commands
      commandQueueCondition_.wait(queuelock,
        [this, &stepsRemaining] {
          return !commandQueue_.empty();
        });
    }
    //  TODO: we may just be able to use a vector for the command queue and
    //        create a local copy of the queue to minimize time spent executing
    //        commands.   the mutex seems to be used just for adds to the
    //        command queue.
    while (!commandQueue_.empty() && isActive) {
      auto command = commandQueue_.front();
      commandQueue_.pop_front();
      switch (command.type) {
        case Command::Terminate:
          isActive = false;
          break;
        case Command::ResetMachine:
          resetMachine();
          isMachineReady = true;
          break;
        case Command::SetHostUpdateFrequency:
          emulatorRefreshFrequency = std::stoul(command.operand);
          if (emulatorRefreshFrequency) {
            fixedFrameInterval =
              std::chrono::microseconds((long)std::floor(1e6/emulatorRefreshFrequency));
          } else {
            fixedFrameInterval = std::chrono::microseconds::zero();
          }
          break;
        case Command::RunMachine:
          stepsRemaining = std::nullopt;
          runSampler.reset();
          break;
        case Command::Publish:
          publishState = true;
          break;
      }
    }
    queuelock.unlock();
    //  if isRunning is false, we use a condition var/wait to hold the thread
    if (!isRunning || !isActive) continue;
    //  TODO: these edge cases seem sloppy - but we'll need to prevent the
    //        thread from spinning if the machine will not run
    if (!isMachineReady || emulatorRefreshFrequency == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(15));
      continue;
    }

    //  Run the emulator in either 'step' or 'run' mode.
    //
    //  RUN MODE executes several instructions in time slices to maximize
    //  performance while providing feedback to the frontend.
    //
    //  STEP MODE executes a single instruction and decrements a 'step' counter
    //
    //  If neither mode is applicable, the emulator holds and this loop will
    //  wait for commands from the frontend
    //
    int64_t clocksPerTimeslice =
      calculateClocksPerTimeslice(&machine_, emulatorRefreshFrequency);
    clocksRemainingInTimeslice += clocksPerTimeslice;
    while (clocksRemainingInTimeslice > 0 &&
           (!stepsRemaining.has_value() || *stepsRemaining > 0)) {

      if (emulatorStepsSinceReset_ >= 2) {
        machine_.cpu.pins.resbIn = true;
      }

      clem_clocks_time_t pre_emulate_time = machine_.clocks_spent;
      clemens_emulate(&machine_);
      clem_clocks_duration_t emulate_step_time = machine_.clocks_spent - pre_emulate_time;

      clocksRemainingInTimeslice -= emulate_step_time;
      if (stepsRemaining.has_value()) {
        *stepsRemaining -= 1;
      }
      ++emulatorStepsSinceReset_;

      //  TODO: MMIO bypass

      //  TODO: check breakpoints, etc

    } // clocksRemainingInTimeslice

    if (stepsRemaining.has_value() && *stepsRemaining == 0) {
      //  if we've finished stepping through code, we are also done with our
      //  timeslice and will wait for a new step/run request
      clocksRemainingInTimeslice = 0;
    }

    auto currentFrameTimePoint = std::chrono::high_resolution_clock::now();
    auto actualFrameInterval = std::chrono::duration_cast<std::chrono::microseconds>(
      currentFrameTimePoint - lastFrameTimePoint);
    lastFrameTimePoint = currentFrameTimePoint;

    runSampler.update(fixedFrameInterval, actualFrameInterval);
    //  TODO: publish emulator state using a callback
    //        the recipient will synchronize with UI accordingly with the
    //        assumption that once the callback returns, we can alter the state
    //        again as needed next timeslice.
    if (publishState) {
      ClemensBackendState publishedState {};
      publishedState.mmio_was_initialized = clemens_is_mmio_initialized(&machine_);
      if (publishedState.mmio_was_initialized) {
        clemens_get_monitor(&publishedState.monitor, &machine_);
        clemens_get_text_video(&publishedState.text, &machine_);
        clemens_get_graphics_video(&publishedState.graphics, &machine_);
        clemens_get_audio(&publishedState.audio, &machine_);
      }
      publishedState.machine = &machine_;
      publishedState.seqno = runSampler.seqNo;
      publishedState.fps = runSampler.sampledFramesPerSecond;
      publishDelegate(publishedState);
      if (publishedState.mmio_was_initialized) {
        clemens_audio_next_frame(&machine_, publishedState.audio.frame_count);
      }
    }
  } // isActive
}

//  Issues a soft reset to the machine.   Pushes the RESB pin low on the 65816
void ClemensBackend::resetMachine() {
  machine_.cpu.pins.resbIn = false;
  emulatorStepsSinceReset_ = 0;
}

cinek::CharBuffer ClemensBackend::loadROM(const char* romPathname) {
  cinek::CharBuffer romBuffer;
  std::ifstream romFileStream(romPathname, std::ios::binary | std::ios::ate);
  unsigned romMemorySize = 0;

  if (romFileStream.is_open()) {
    romMemorySize = unsigned(romFileStream.tellg());
    romBuffer = cinek::CharBuffer(
      (char *)slabMemory_.allocate(romMemorySize), romMemorySize);
    romFileStream.seekg(0, std::ios::beg);
    romFileStream.read(romBuffer.forwardSize(romMemorySize).first, romMemorySize);
    romFileStream.close();
  }
  return romBuffer;
}

void ClemensBackend::initApple2GS() {
  const unsigned kFPIBankCount = CLEM_IIGS_FPI_MAIN_RAM_BANK_COUNT;
  const uint32_t kClocksPerFastCycle = CLEM_CLOCKS_FAST_CYCLE;
  const uint32_t kClocksPerSlowCycle = CLEM_CLOCKS_MEGA2_CYCLE;
  clemens_init(&machine_, kClocksPerSlowCycle, kClocksPerFastCycle,
               romBuffer_.getHead(), romBuffer_.getSize(),
               slabMemory_.allocate(CLEM_IIGS_BANK_SIZE),
               slabMemory_.allocate(CLEM_IIGS_BANK_SIZE),
               slabMemory_.allocate(CLEM_IIGS_BANK_SIZE * kFPIBankCount),
               slabMemory_.allocate(2048 * 7),    //  TODO: placeholder
               kFPIBankCount);
  loadBRAM();

  //  TODO: It seems the internal audio code expects 2 channel float PCM,
  //        so we only need buffer size and frequency.
  ClemensAudioMixBuffer audioMixBuffer;
  audioMixBuffer.frames_per_second = config_.audioSamplesPerSecond;
  audioMixBuffer.stride = 2 * sizeof(float);
  audioMixBuffer.frame_count = audioMixBuffer.frames_per_second / 4;
  audioMixBuffer.data = (uint8_t*)(
    slabMemory_.allocate(audioMixBuffer.frame_count * audioMixBuffer.stride)
  );
  clemens_assign_audio_mix_buffer(&machine_, &audioMixBuffer);
}

void ClemensBackend::saveBRAM() {
  bool isDirty = false;
  const uint8_t *bram = clemens_rtc_get_bram(&machine_, &isDirty);
  if (!isDirty)
    return;

  std::ofstream bramFile("clem.bram", std::ios::binary);
  if (bramFile.is_open()) {
    bramFile.write((char *)machine_.mmio.dev_rtc.bram, CLEM_RTC_BRAM_SIZE);
  } else {
    //  TODO: display error?
  }
}

void ClemensBackend::loadBRAM() {
  std::ifstream bramFile("clem.bram", std::ios::binary);
  if (bramFile.is_open()) {
    bramFile.read((char *)machine_.mmio.dev_rtc.bram, CLEM_RTC_BRAM_SIZE);
  } else {
    //  TODO: display warning?
  }
}

void ClemensBackend::emulatorLog(int log_level, ClemensMachine *machine,
                                 const char *msg) {
  auto *host = reinterpret_cast<ClemensBackend *>(machine->debug_user_ptr);
}

//  If enabled, this emulator issues this callback per instruction
//  This is great for debugging but should be disabled otherwise
void ClemensBackend::emulatorOpcodeCallback(struct ClemensInstruction *inst,
                                            const char *operand, void *this_ptr) {
  auto *host = reinterpret_cast<ClemensBackend *>(this_ptr);
}
