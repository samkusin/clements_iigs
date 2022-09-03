#ifndef CLEM_HOST_BACKEND_HPP
#define CLEM_HOST_BACKEND_HPP

#include "clem_host_shared.hpp"

#include "cinek/fixedstack.hpp"
#include "cinek/buffer.hpp"

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>


//  TODO: Machine type logic could be subclassed into an Apple2GS backend, etc.
class ClemensBackend {
public:
  enum class Type {
    Apple2GS
  };
  struct Config {
    unsigned audioSamplesPerSecond;
    Type type;
  };
  using PublishStateDelegate = std::function<void(const ClemensBackendState&)>;
  ClemensBackend(std::string romPathname, const Config& config,
                 PublishStateDelegate publishDelegate);
  ~ClemensBackend();


  //  Queues a command to the backend.   Most commands are processed on the next
  //  execution frame.  Certain commands may hold the queue (i.e. like wait())
  //  Priority commands like Cancel or Terminate are pushed to the front,
  //  overriding commands like Wait.
  void terminate();
  //  Issues a soft reset to the machine.   This is roughly equivalent to pressing
  //  the power button.
  void reset();
  //  The host should expect emulator state refreshes at this frequency if in
  //  run mode
  void setRefreshFrequency(unsigned hz);
  //  Clears step mode and enter run mode
  void run();
  //  Will issue the publish delegate on the next machine iteration
  void publish();
  //  Send host input to the emulator
  void inputEvent(const ClemensInputEvent& inputEvent);

private:
  struct Command {
    enum Type {
      Undefined,
      Terminate,
      SetHostUpdateFrequency,
      ResetMachine,
      RunMachine,
      Publish,
      Input
    };
    Type type = Undefined;
    std::string operand;
  };

  void queue(const Command& cmd);
  void queueToFront(const Command& cmd);

  void main(PublishStateDelegate publishDelegate);
  void resetMachine();
  void inputMachine(const std::string_view& inputParam);

  cinek::CharBuffer loadROM(const char* romPathname);
  //  TODO: These methods could be moved into a subclass as they are specific
  //        to machine type
  void initApple2GS();
  void loadBRAM();
  void saveBRAM();

  static void emulatorLog(int log_level, ClemensMachine *machine,
                          const char *msg);
  static void emulatorOpcodeCallback(struct ClemensInstruction *inst,
                                     const char *operand, void *this_ptr);

private:
  Config config_;

  std::thread runner_;
  std::deque<Command> commandQueue_;
  std::mutex commandQueueMutex_;
  std::condition_variable commandQueueCondition_;

  //  memory allocated once for the machine
  cinek::FixedStack slabMemory_;
  //  the actual machine object
  cinek::CharBuffer romBuffer_;
  ClemensMachine machine_;

  std::vector<ClemensBackendOutputText> logOutput_;
};

#endif
