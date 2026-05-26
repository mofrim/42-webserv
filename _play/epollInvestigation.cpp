// conclusion:
//
// - if the readEnd in the child is being closed, we get EPOLLOUT & EPOLLERR on
// the parentWrite FD
//
// - if the writeEnd in the child close, we get at least EPOLLHUP on the
// parentRead FD. if there is also EPOLLIN this means there is still data to
// read.

#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

typedef std::string str;

str getEventStr(const uint32_t& ev)
{
  str ret;

  if (ev & EPOLLIN)
    ret += str("EPOLLIN ");
  if (ev & EPOLLOUT)
    ret += str("EPOLLOUT ");
  if (ev & EPOLLERR)
    ret += str("EPOLLERR ");
  if (ev & EPOLLHUP)
    ret += str("EPOLLHUP ");
  return ret;
}

typedef struct {
    int   epollFd;
    int   pRead;
    int   pWrite;
    pid_t cpid;
} t_epollInv;

enum e_whichEnd { WRITE, READ };

t_epollInv setStage(e_whichEnd e)
{
  t_epollInv ret;
  ret.epollFd = epoll_create1(EPOLL_CLOEXEC);

  int p[2];
  if (pipe(p) == -1) {
    std::cout << "pipe failed" << std::endl;
    _exit(1);
  }

  ret.pRead  = p[0];
  ret.pWrite = p[1];

  if (e == WRITE) {
    struct epoll_event eRead = {.events = EPOLLIN, .data = {.fd = ret.pRead}};
    epoll_ctl(ret.epollFd, EPOLL_CTL_ADD, ret.pRead, &eRead);
  }
  else {
    struct epoll_event eWrite = {
        .events = EPOLLOUT, .data = {.fd = ret.pWrite}};
    epoll_ctl(ret.epollFd, EPOLL_CTL_ADD, ret.pWrite, &eWrite);
  }

  return ret;
}

void monitorChild(e_whichEnd e, t_epollInv ei)
{
  if (e == WRITE)
    close(ei.pWrite);
  else
    close(ei.pRead);

  int wret = waitpid(ei.cpid, 0, WNOHANG);
  if (wret == 0)
    std::cout << "child is running!" << std::endl;

  struct epoll_event events[5];
  bool               oneMore = true;
  while (wret == 0 || oneMore) {
    if (wret != 0)
      oneMore = false;
    int nfds = epoll_wait(ei.epollFd, events, 5, -1);
    std::cout << "epoll_wait returned with nfds = " << nfds << std::endl;
    for (int i = 0; i < nfds; ++i)
      std::cout << "event no." << i << ": " << getEventStr(events[i].events)
                << std::endl;
    usleep(500000);
    wret = waitpid(ei.cpid, 0, WNOHANG);
    if (wret != 0)
      std::cout << "waitpid return: " << wret << std::endl;
  }
}

void closeAllFds(t_epollInv ei)
{
  close(ei.epollFd);
  close(ei.pRead);
  close(ei.pWrite);
}

void closeWriteEnd()
{
  t_epollInv ei = setStage(WRITE);

  ei.cpid = fork();

  if (ei.cpid == 0) {
    dup2(ei.pWrite, STDOUT_FILENO);
    close(ei.pRead);
    close(ei.pWrite);
    (void)write(STDOUT_FILENO, "42", 2);
    sleep(1);
    close(STDOUT_FILENO);
    sleep(1);
    _exit(0);
  }

  monitorChild(WRITE, ei);
  closeAllFds(ei);
}

void exitWriteEnd()
{
  t_epollInv ei = setStage(WRITE);

  ei.cpid = fork();

  if (ei.cpid == 0) {
    dup2(ei.pWrite, STDOUT_FILENO);
    close(ei.pRead);
    close(ei.pWrite);
    (void)write(STDOUT_FILENO, "42", 2);
    sleep(1);
    _exit(1);
  }

  monitorChild(WRITE, ei);
  closeAllFds(ei);
}

void closeReadEnd()
{
  t_epollInv ei = setStage(READ);

  ei.cpid = fork();

  if (ei.cpid == 0) {
    dup2(ei.pRead, STDIN_FILENO);
    close(ei.pRead);
    close(ei.pWrite);
    sleep(1);
    close(STDIN_FILENO);
    sleep(1);
    _exit(0);
  }

  monitorChild(READ, ei);
  closeAllFds(ei);
}

void exitReadEnd()
{
  t_epollInv ei = setStage(READ);

  ei.cpid = fork();

  if (ei.cpid == 0) {
    dup2(ei.pRead, STDIN_FILENO);
    close(ei.pRead);
    close(ei.pWrite);
    sleep(1);
    _exit(1);
  }

  monitorChild(READ, ei);
  closeAllFds(ei);
}

int main()
{
  std::cout << "close child's read end:\n----------------" << std::endl;
  closeReadEnd();
  std::cout << "\nkill child's read end:\n----------------" << std::endl;
  exitReadEnd();
  std::cout << "\nclose child's write end:\n----------------" << std::endl;
  closeWriteEnd();
  std::cout << "\nkill childs's write end:\n----------------" << std::endl;
  exitWriteEnd();
  return 0;
}

// void closeWriteEnd()
// {
//   int _epollFd = epoll_create1(EPOLL_CLOEXEC);
//
//   int p[2];
//   if (pipe(p) == -1) {
//     std::cout << "pipe failed" << std::endl;
//     return;
//   }
//
//   int pRead  = p[0];
//   int pWrite = p[1];
//
//   struct epoll_event eRead = {.events = EPOLLIN, .data = {.fd = pRead}};
//   epoll_ctl(_epollFd, EPOLL_CTL_ADD, pRead, &eRead);
//
//   int cpid = fork();
//
//   if (cpid == 0) {
//     dup2(pWrite, STDOUT_FILENO);
//     close(pRead);
//     close(pWrite);
//     sleep(1);
//     close(STDOUT_FILENO);
//     sleep(1);
//     _exit(0);
//   }
//
//   close(pWrite);
//
//   int wret = waitpid(cpid, 0, WNOHANG);
//   if (wret == 0)
//     std::cout << "child is running!" << std::endl;
//
//   struct epoll_event events[5];
//   bool               oneMore = true;
//   while (wret == 0 || oneMore) {
//     if (wret != 0)
//       oneMore = false;
//     int nfds = epoll_wait(_epollFd, events, 5, -1);
//     std::cout << "epoll_wait returned with nfds = " << nfds << std::endl;
//     for (int i = 0; i < nfds; ++i)
//       std::cout << "event no." << i << ": " << getEventStr(events[i].events)
//                 << std::endl;
//     usleep(500000);
//     wret = waitpid(cpid, 0, WNOHANG);
//     if (wret != 0)
//       std::cout << "wret: " << wret << " - child reaped" << std::endl;
//   }
// }
