// Moemon VMU manager: the VM2 from the command line (the same code the manager uses; tests and quick jobs).
//   vm2cli ports
//   vm2cli [--port P] info | clock | ls [FOLDER] | get PATH OUT | put LOCAL PATH | cp A B | mv A B | rm PATH
//          | mkdir F | rmdir F | readint OUT | writeint IN
// Paths as on the VM2: \NAME.BIN, \FOLDER\NAME.BIN; folders \ or \FOLDER.
#include "vm2link.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>

static vm2::Bytes slurp(const char* p) {
    std::ifstream f(p, std::ios::binary);
    return vm2::Bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

static bool spit(const char* p, const vm2::Bytes& b) {
    std::ofstream f(p, std::ios::binary);
    f.write((const char*)b.data(), (std::streamsize)b.size());
    return (bool)f;
}

int main(int argc, char** argv) {
    std::string port;
    int a = 1;
    if(a + 1 < argc && !strcmp(argv[a], "--port")) { port = argv[a + 1]; a += 2; }
    if(a >= argc) { fprintf(stderr, "usage: vm2cli [--port P] ports|info|clock|ls|get|put|cp|mv|rm|mkdir|rmdir|readint|writeint ...\n"); return 2; }
    std::string cmd = argv[a++];
    if(cmd == "ports") {
        for(auto& p : vm2::listPorts()) printf("%s\t%s%s\n", p.name.c_str(), p.description.c_str(), p.ch340 ? "  <- VM2?" : "");
        return 0;
    }
    vm2::Device dev;
    std::string err;
    if(!dev.connect(port, &err)) { fprintf(stderr, "%s\n", err.c_str()); return 1; }
    auto arg = [&](int i) -> const char* { return a + i < argc ? argv[a + i] : ""; };
    bool ok = true;
    if(cmd == "info") printf("port %s at %d baud\nhardware %s\nfirmware %s\ndevice %s\n", dev.port.c_str(), dev.baud,
                             dev.hw.c_str(), dev.fw.c_str(), dev.id.c_str());
    else if(cmd == "clock") ok = dev.syncClock(&err);
    else if(cmd == "ls") {
        std::vector<vm2::Entry> es;
        ok = dev.list(*arg(0) ? arg(0) : "\\", es, &err);
        for(auto& e : es) printf("%s\t%s\t%u\t%s\n", e.dir ? "DIR " : "FILE", e.name.c_str(), e.size, e.title.c_str());
    } else if(cmd == "get") {
        vm2::Bytes b;
        ok = dev.readFile(arg(0), 0, b, nullptr, &err) && spit(arg(1), b);
    } else if(cmd == "put") ok = dev.writeFile(arg(1), slurp(arg(0)), nullptr, &err);
    else if(cmd == "cp") ok = dev.copyFile(arg(0), arg(1), &err);
    else if(cmd == "mv") ok = dev.rename(arg(0), arg(1), &err);
    else if(cmd == "rm") ok = dev.removeFile(arg(0), &err);
    else if(cmd == "mkdir") ok = dev.makeFolder(arg(0), &err);
    else if(cmd == "rmdir") ok = dev.removeFolder(arg(0), &err);
    else if(cmd == "readint") {
        vm2::Bytes b;
        ok = dev.readInternal(b, nullptr, &err) && spit(arg(0), b);
    } else if(cmd == "writeint") ok = dev.writeInternal(slurp(arg(0)), nullptr, &err);
    else { fprintf(stderr, "unknown command %s\n", cmd.c_str()); return 2; }
    if(!ok) { fprintf(stderr, "%s\n", err.c_str()); return 1; }
    return 0;
}
