#include "game_wait_history_runtime.hpp"
#include <petari/game_wait_history.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>
namespace H=PetariNative::GameWaitHistory;
void check(bool ok,const char* why){if(!ok){std::fprintf(stderr,"FAIL %s\n",why);std::exit(1);}}
int main(int argc,char** argv){
    auto pattern=(std::filesystem::temp_directory_path()/"petari-game-wait-XXXXXX").string();
    const char* temp=::mkdtemp(pattern.data());check(temp,"private root");
    const std::filesystem::path root(temp),file=root/"history.jsonl";
    ::unsetenv("PETARI_GAME_WAIT_HISTORY");::setenv("PETARI_GAME_WAIT_HISTORY_FILE",file.c_str(),1);
    check(H::initializeFromEnvironment()&&!std::filesystem::exists(file)&&!H::enabled.load(),"OFF creates nothing");
    if(argc==2){
        {std::ofstream out(file);out<<"preserve";}
        ::setenv("PETARI_GAME_WAIT_HISTORY","1",1);
        check(!H::initializeFromEnvironment()&&!H::enabled.load(),"existing file refused");
        std::ifstream in(file);std::string value;in>>value;check(value=="preserve","existing evidence untouched");in.close();
        std::filesystem::remove(file);std::filesystem::remove(root);return 0;
    }
    const auto child=::fork();check(child>=0,"fork before observers");
    if(!child){::execl(argv[0],argv[0],"refuse",static_cast<char*>(nullptr));::_exit(127);}
    int status=0;check(::waitpid(child,&status,0)==child&&WIFEXITED(status)&&!WEXITSTATUS(status),"refusal subprocess");
    ::setenv("PETARI_GAME_WAIT_HISTORY","1",1);
    check(H::initializeFromEnvironment()&&H::enabled.load(),"explicit opt-in enabled");
    // Establish two seconds of real prehistory; no application, disc or cache.
    for(unsigned i=0;i<140;++i){H::mark(0,i+1);H::mark(1,i+1);std::this_thread::sleep_for(std::chrono::milliseconds(16));}
    H::mark(0,141);
    H::invoke(H::Kind::Token,123,0,[]{std::this_thread::sleep_for(std::chrono::milliseconds(140));});
    H::mark(1,141);
    for(unsigned i=0;i<30;++i){H::mark(0,i+142);H::mark(1,i+142);std::this_thread::sleep_for(std::chrono::milliseconds(16));}
    H::finish();H::finish();check(!H::enabled.load(),"finish idempotent and disables producers");
    std::ifstream in(file);const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());in.close();
    check(text.find("\"type\":\"header\"")!=std::string::npos&&text.find("\"type\":\"footer\"")!=std::string::npos,"orderly protocol footer");
    check(text.find("\"type\":\"summary\"")!=std::string::npos&&text.find("\"type\":\"window\"")!=std::string::npos,"actual off-CPU sleep triggers window");
    check(text.size()<=64ull*1024*1024,"hard output cap");
    std::filesystem::remove(file);std::filesystem::remove(root);
    std::puts("game wait runtime passed (private root, no app or cache)");
}
