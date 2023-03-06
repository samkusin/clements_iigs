unsigned char fast_emulate_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x40,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x5c, 0x20, 0xf4, 0x46, 0x00, 0x00, 0x00,
  0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x01, 0xd8, 0x00, 0x00, 0x01,
  0xd8, 0x01, 0xfa, 0x5c, 0xa6, 0x72, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45,
  0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x77,
  0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63, 0x61, 0x70, 0x65, 0x2e,
  0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00, 0x00, 0x03, 0x39, 0x49,
  0x44, 0x41, 0x54, 0x68, 0x81, 0xd5, 0x9a, 0x3b, 0x68, 0x14, 0x51, 0x14,
  0x86, 0xbf, 0x5d, 0x43, 0xea, 0xd8, 0x06, 0xc5, 0xf5, 0xdd, 0x28, 0x82,
  0xa5, 0x20, 0xae, 0x0a, 0x16, 0x56, 0xf9, 0x1b, 0x0b, 0x0b, 0xa3, 0x82,
  0x10, 0x0b, 0x9b, 0x88, 0x88, 0x46, 0xac, 0x34, 0xda, 0xf8, 0xec, 0xc4,
  0x17, 0x22, 0xf1, 0x59, 0xdc, 0x26, 0x06, 0x44, 0x82, 0x36, 0x3e, 0xd0,
  0x22, 0x42, 0x94, 0x28, 0xf8, 0x08, 0x24, 0xf8, 0x08, 0xf8, 0x0a, 0x8a,
  0x1a, 0xd7, 0xac, 0xc5, 0x66, 0x65, 0xb3, 0x6e, 0xb2, 0x33, 0x73, 0xcf,
  0xce, 0xb8, 0x7f, 0xb7, 0x33, 0xe7, 0xfe, 0xe7, 0x7e, 0x7b, 0xce, 0xde,
  0x99, 0x3b, 0xb3, 0xa9, 0x96, 0x96, 0x16, 0x4a, 0xe5, 0x9c, 0x4b, 0x01,
  0xad, 0xc0, 0x0e, 0x60, 0x09, 0xd0, 0x88, 0xad, 0x72, 0xc0, 0x00, 0x70,
  0x58, 0xd2, 0x25, 0x5f, 0xb3, 0x74, 0x85, 0x63, 0x07, 0x80, 0xf3, 0xc0,
  0x72, 0xec, 0x27, 0x0f, 0xd0, 0x00, 0x2c, 0x05, 0xba, 0x9c, 0x73, 0x1b,
  0x7d, 0xcd, 0x26, 0x01, 0x38, 0xe7, 0x32, 0xc0, 0x1e, 0x5f, 0xd3, 0x10,
  0xda, 0xed, 0x6b, 0x50, 0x5e, 0x81, 0xf5, 0x40, 0xca, 0xd7, 0x34, 0x84,
  0xe6, 0xf9, 0x1a, 0x94, 0x03, 0x2c, 0xf4, 0x35, 0x0c, 0xa9, 0xa7, 0xbe,
  0x06, 0xe5, 0x00, 0x19, 0x5f, 0xc3, 0x90, 0xba, 0xea, 0x6b, 0x90, 0x24,
  0x40, 0x0e, 0xb8, 0xec, 0x6b, 0x92, 0x24, 0x40, 0xb7, 0xa4, 0x77, 0xbe,
  0x26, 0x7f, 0x01, 0x9c, 0x73, 0x4d, 0x40, 0x93, 0xaf, 0x61, 0x08, 0x5d,
  0xb0, 0x30, 0x29, 0xad, 0x40, 0xc6, 0xc2, 0x30, 0xa0, 0x3e, 0x00, 0x3d,
  0x16, 0x46, 0x49, 0x01, 0x5c, 0x94, 0x34, 0x66, 0x61, 0x94, 0x14, 0x80,
  0x49, 0xfb, 0xc0, 0x64, 0x80, 0x39, 0x56, 0xa6, 0x55, 0xd4, 0x2f, 0xe9,
  0xb1, 0x95, 0x59, 0x12, 0x15, 0x38, 0x67, 0x69, 0x16, 0x37, 0x80, 0xc9,
  0xda, 0x5f, 0xaa, 0xb8, 0x01, 0xba, 0x25, 0xbd, 0xb7, 0x34, 0x4c, 0x43,
  0xac, 0xd7, 0x00, 0xb3, 0x1f, 0x6f, 0x51, 0xc5, 0x0a, 0x64, 0xac, 0x8d,
  0x2b, 0xc8, 0x6c, 0xed, 0x2f, 0x55, 0x9c, 0x00, 0x66, 0x6b, 0x7f, 0xa9,
  0xe2, 0x04, 0x30, 0x6f, 0x1f, 0x28, 0x6c, 0xef, 0x20, 0x9e, 0x6b, 0x40,
  0x9f, 0x73, 0xce, 0xd7, 0x23, 0x07, 0xbc, 0xa2, 0xb0, 0xe5, 0x3d, 0x2a,
  0x69, 0x2c, 0xce, 0x0a, 0x58, 0xa8, 0x01, 0x58, 0x04, 0x1c, 0x02, 0x7a,
  0x9c, 0x73, 0x8d, 0xf5, 0x06, 0x50, 0xaa, 0xb5, 0x40, 0x7b, 0x3d, 0x03,
  0x00, 0xb4, 0xa6, 0xf2, 0xf9, 0x7c, 0x13, 0xf0, 0x29, 0xe9, 0x99, 0x44,
  0xd4, 0x8f, 0x34, 0xf5, 0xfb, 0xed, 0x03, 0x0c, 0xd7, 0x3b, 0xc0, 0xb5,
  0x7a, 0x06, 0x78, 0x0b, 0x1c, 0x49, 0x13, 0xdf, 0x3e, 0xc0, 0x5a, 0xdb,
  0x24, 0x7d, 0xac, 0xd7, 0x0a, 0x9c, 0x91, 0x74, 0x03, 0x0a, 0xb7, 0x12,
  0x99, 0x64, 0xe7, 0x12, 0x5a, 0x83, 0xc0, 0xce, 0xe2, 0x87, 0x7a, 0x03,
  0x18, 0x07, 0xb6, 0x48, 0x1a, 0x2d, 0x1e, 0x48, 0x03, 0x33, 0x92, 0x9b,
  0x4f, 0x68, 0x9d, 0x90, 0x74, 0xa7, 0xf4, 0x40, 0x1a, 0xe8, 0x4d, 0x66,
  0x2e, 0xa1, 0x35, 0x00, 0x74, 0x94, 0x1f, 0x4c, 0xe5, 0xf3, 0xf9, 0xc5,
  0xc0, 0x7d, 0x60, 0x66, 0xec, 0x53, 0x0a, 0xae, 0x1c, 0xb0, 0x42, 0xd2,
  0xa3, 0xf2, 0x13, 0x69, 0x49, 0xcf, 0x81, 0x65, 0xc0, 0x75, 0x60, 0xb4,
  0x3c, 0xe0, 0x3f, 0x51, 0x67, 0xa5, 0xc9, 0xc3, 0xc4, 0x7e, 0x40, 0xd2,
  0x10, 0xb0, 0xc1, 0x2a, 0xdb, 0xc4, 0x9b, 0x9e, 0xd7, 0x46, 0x76, 0x7d,
  0x14, 0x5e, 0x7b, 0x55, 0x54, 0xa5, 0x77, 0x64, 0x16, 0xca, 0x1a, 0xf9,
  0xfc, 0x04, 0x5a, 0x25, 0xfd, 0x9a, 0x2a, 0xa0, 0x56, 0x00, 0xab, 0x8c,
  0x7c, 0x3a, 0x24, 0xf5, 0x4f, 0x17, 0xf0, 0x3f, 0x57, 0xe0, 0x2e, 0x70,
  0xbc, 0x5a, 0x90, 0x39, 0x80, 0x73, 0x6e, 0x36, 0xfe, 0x17, 0xc7, 0x6f,
  0xc0, 0x66, 0x49, 0xbf, 0xab, 0x05, 0xd6, 0xa2, 0x02, 0x6b, 0x0c, 0x3c,
  0xda, 0x25, 0xbd, 0x08, 0x12, 0x58, 0x0b, 0x00, 0xdf, 0xfe, 0xbf, 0x05,
  0x9c, 0x0e, 0x1a, 0x5c, 0x0b, 0x80, 0xac, 0xc7, 0xd8, 0xcf, 0xc0, 0x56,
  0x49, 0xf9, 0xa0, 0x03, 0x4c, 0x01, 0x26, 0xfa, 0x7f, 0xae, 0x87, 0xc5,
  0x76, 0x49, 0xc3, 0x61, 0x06, 0x58, 0x57, 0x60, 0xb5, 0xc7, 0x58, 0x27,
  0xe9, 0x4a, 0xd8, 0x41, 0xd6, 0x00, 0x51, 0xfb, 0x7f, 0x04, 0x68, 0x8b,
  0x32, 0xd0, 0x1a, 0x20, 0x1b, 0x71, 0x5c, 0x9b, 0xa4, 0x91, 0x28, 0x03,
  0xcd, 0x00, 0x9c, 0x73, 0xb3, 0x88, 0xf6, 0xe7, 0x8d, 0xb3, 0x92, 0x22,
  0x3f, 0x34, 0xb5, 0xac, 0x40, 0x94, 0xfe, 0x1f, 0x04, 0xda, 0x7d, 0x92,
  0x5a, 0x02, 0x84, 0xed, 0xff, 0x71, 0x0a, 0x4b, 0xa6, 0xd7, 0x2d, 0xbc,
  0x25, 0x40, 0x36, 0x64, 0xfc, 0x49, 0x49, 0xb7, 0x7d, 0x93, 0x9a, 0x00,
  0x4c, 0xf4, 0xff, 0xfc, 0x10, 0x43, 0x9e, 0x01, 0x7b, 0x2d, 0x72, 0x5b,
  0x55, 0x20, 0x1b, 0x22, 0x36, 0x47, 0xe1, 0x1e, 0xff, 0xbb, 0x45, 0x62,
  0x2b, 0x80, 0x30, 0xfd, 0xdf, 0x29, 0xe9, 0xa1, 0x51, 0xde, 0xd8, 0x2b,
  0xd0, 0x07, 0x1c, 0x34, 0xca, 0x09, 0x18, 0x00, 0x38, 0xe7, 0x9a, 0x81,
  0x05, 0x01, 0x42, 0x8b, 0xdb, 0x43, 0xd3, 0x37, 0x95, 0x16, 0x15, 0x08,
  0xba, 0xfe, 0xef, 0xab, 0xb6, 0x3d, 0x8c, 0x22, 0x0b, 0x80, 0x20, 0xfd,
  0x7f, 0x0f, 0x38, 0x66, 0x90, 0xeb, 0x1f, 0x59, 0x00, 0xac, 0xac, 0x72,
  0xfe, 0x2b, 0xb0, 0x29, 0xc8, 0xf6, 0x30, 0x8a, 0x2c, 0x00, 0x9a, 0xab,
  0x9c, 0xdf, 0x25, 0xe9, 0xa5, 0x41, 0x9e, 0x8a, 0xb2, 0x00, 0x78, 0x32,
  0xcd, 0xb9, 0x9b, 0xc0, 0x29, 0x83, 0x1c, 0x53, 0xca, 0x02, 0x60, 0x3f,
  0x50, 0x69, 0x0b, 0x38, 0x44, 0xc8, 0xed, 0x61, 0x14, 0x79, 0x03, 0x48,
  0xea, 0x05, 0xd6, 0x01, 0x0f, 0x28, 0x2c, 0x95, 0x5f, 0x80, 0x2e, 0x0a,
  0x0f, 0x63, 0xdf, 0xf8, 0xfa, 0x57, 0xd3, 0x1f, 0x9b, 0x8a, 0xba, 0x42,
  0x36, 0xf5, 0x3a, 0x89, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
  0xae, 0x42, 0x60, 0x82
};
unsigned int fast_emulate_png_len = 940;
