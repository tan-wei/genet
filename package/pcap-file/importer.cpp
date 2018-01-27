#include <fstream>
#include <plugkit/context.h>
#include <plugkit/file.h>
#include <plugkit/reader.h>
#include <vector>

using namespace plugkit;

FileStatus import(Context *ctx,
                  const char *filename,
                  RawFrame *dst,
                  size_t capacity,
                  FileImporterCallback callback) {

  std::ifstream ifs(filename, std::ios::binary);
  if (!ifs.is_open()) {
    return FILE_STATUS_ERROR;
  }
  char header[24];
  ifs.read(header, sizeof header);
  if (!ifs) {
    return FILE_STATUS_UNSUPPORTED;
  }

  Reader headerReader;
  Reader_reset(&headerReader);

  headerReader.data.begin = header;
  headerReader.data.end = header + sizeof header;

  bool littleEndian = true;
  bool nanosec = false;
  uint32_t magicNumber = Reader_getUint32(&headerReader, false);
  switch (magicNumber) {
  case 0xd4c3b2a1:
    littleEndian = true;
    nanosec = false;
    break;
  case 0xa1b2c3d4:
    littleEndian = false;
    nanosec = false;
    break;
  case 0x4d3cb2a1:
    littleEndian = true;
    nanosec = true;
    break;
  case 0xa1b23c4d:
    littleEndian = false;
    nanosec = true;
    break;
  default:
    return FILE_STATUS_UNSUPPORTED;
  }

  uint16_t versionMajor = Reader_getUint16(&headerReader, littleEndian);
  uint16_t versionMinor = Reader_getUint16(&headerReader, littleEndian);
  int32_t thiszone = Reader_getInt32(&headerReader, littleEndian);
  uint32_t sigfigs = Reader_getUint32(&headerReader, littleEndian);
  uint32_t snaplen = Reader_getUint32(&headerReader, littleEndian);
  uint32_t network = Reader_getUint32(&headerReader, littleEndian);

  size_t headerOffset = ifs.tellg();
  ifs.seekg(0, ifs.end);
  size_t dataLength = ifs.tellg();
  ifs.seekg(headerOffset, ifs.beg);

  while (ifs) {
    size_t count = 0;

    for (count = 0; count < capacity; ++count) {
      char frameHeader[16];
      ifs.read(frameHeader, sizeof frameHeader);
      if (!ifs) {
        break;
      }

      Reader frameReader;
      Reader_reset(&frameReader);

      frameReader.data.begin = frameHeader;
      frameReader.data.end = frameHeader + sizeof frameHeader;

      uint32_t tsSec = Reader_getUint32(&frameReader, littleEndian);
      uint32_t tsUsec = Reader_getUint32(&frameReader, littleEndian);
      uint32_t inclLen = Reader_getUint32(&frameReader, littleEndian);
      uint32_t origLen = Reader_getUint32(&frameReader, littleEndian);

      char *buffer = static_cast<char *>(Context_alloc(ctx, inclLen));
      ifs.read(buffer, inclLen);
      if (!ifs) {
        break;
      }

      if (!nanosec) {
        tsUsec *= 1000;
      }

      RawFrame &frame = dst[count];
      frame.link = network;
      frame.data = buffer;
      frame.length = inclLen;
      frame.actualLength = origLen;
      frame.tsSec = tsSec;
      frame.tsNsec = tsUsec;
    }

    callback(ctx, count, 1.0 * ifs.tellg() / dataLength);
  }

  callback(ctx, 0, 1.0);
  return FILE_STATUS_DONE;
}

extern "C" {

PLUGKIT_MODULE_EXPORT FileStatus
plugkit_v1_file_import(Context *ctx,
                       const char *filename,
                       RawFrame *dst,
                       size_t capacity,
                       FileImporterCallback callback) {
  return import(ctx, filename, dst, capacity, callback);
}
}
