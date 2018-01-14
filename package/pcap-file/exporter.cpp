#include <fstream>
#include <plugkit/context.h>
#include <plugkit/file.h>
#include <plugkit/module.h>
#include <plugkit/variant.h>

using namespace plugkit;

FileStatus
exportFile(Context *ctx, const char *filename, FileExporterCallback callback) {
  std::ofstream ofs(filename, std::ios::binary);
  if (!ofs.is_open()) {
    return FILE_STATUS_ERROR;
  }

  const unsigned char magicNumber[] = {0x4d, 0x3c, 0xb2, 0xa1};
  ofs.write(reinterpret_cast<const char *>(magicNumber), sizeof(magicNumber));

  bool header = false;

  while (1) {
    size_t length;
    const RawFrame *begin = callback(ctx, &length);
    if (length == 0)
      break;

    if (!header) {
      header = true;
      uint16_t versionMajor = 2;
      uint16_t versionMinor = 4;
      int32_t thiszone = 0;
      uint32_t sigfigs = 0;
      uint32_t snaplen =
          Variant_uint32(Context_getOption(ctx, "_.pcap.snaplen"));
      uint32_t network = begin[0].link;
      ofs.write(reinterpret_cast<const char *>(&versionMajor),
                sizeof(versionMajor));
      ofs.write(reinterpret_cast<const char *>(&versionMinor),
                sizeof(versionMinor));
      ofs.write(reinterpret_cast<const char *>(&thiszone), sizeof(thiszone));
      ofs.write(reinterpret_cast<const char *>(&sigfigs), sizeof(sigfigs));
      ofs.write(reinterpret_cast<const char *>(&snaplen), sizeof(snaplen));
      ofs.write(reinterpret_cast<const char *>(&network), sizeof(network));
    }

    for (size_t i = 0; i < length; ++i) {
      uint32_t tsSec = begin[i].tsSec;
      uint32_t tsUsec = begin[i].tsNsec;
      uint32_t inclLen = begin[i].length;
      uint32_t origLen = begin[i].actualLength;
      ofs.write(reinterpret_cast<const char *>(&tsSec), sizeof(tsSec));
      ofs.write(reinterpret_cast<const char *>(&tsUsec), sizeof(tsUsec));
      ofs.write(reinterpret_cast<const char *>(&inclLen), sizeof(inclLen));
      ofs.write(reinterpret_cast<const char *>(&origLen), sizeof(origLen));
      ofs.write(begin[i].data, inclLen);
    }
  }

  return FILE_STATUS_DONE;
}

PLUGKIT_MODULE("exporter", []() {
  static FileExporter exporter;
  exporter.func = exportFile;
  return &exporter;
})
