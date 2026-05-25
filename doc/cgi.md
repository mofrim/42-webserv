# The CGI Problem

These are the possible scenarios for a CGI request:

1) we are still writing and not everything has been written but receive
    EPOLLERR on the write fd. This means we could not write the whole reqBody
    to the pipe -> 504. What must be handled?

    - a 504 response can be send to the client immediately (no conn close!)
    - the child must be killed and/or reaped
    - if the client also disconnected (how to check that?)  -> disco client
    - disco client means: only after the child is completely gone remove the
      client from interest list and destroy

2) we still writing and we receive EPOLLHUP on the read fd. this would be
    the same.

3) we are done writing and receive a EPOLLHUP on the read fd. we should check
    child's exit status. if it exited with non-zero error or has not yet exited
    there is probably an error. can this also happen if a client disconnects and
    the first fd handled from epoll ready-list is by chance a pipe-fd?

    If child exited with 0 , then we will have to keep reading from the pipe
    until we get a read == 0. Then we are fine and processing can continue as
    usual.

    Assumption here: The CGI is responsible for sending what it wants to send.
    If the child-response is incomplete it is not our fault.

4) the normal case: writing & reading is completed successfully, we still will
   have to handle and check that child process exits correctly. What to do if
   child process exits with a non-zero exit-status even though write & read went
   well?

So there is still to clarify how to handle the child killing. Should we
continue processing the request while the child is not yet killed? meaning, we
delay it until clients state is `CLI_CGIOK`? Or shall we do it asynchronously
outside the main if-else-epoll-branch?

The child exit-status must be checkable outside of any child-killing routine.
This means if `Response::cgiEvalChildState()` gets a -1 or a non-zero
exit-status we set a state, proly better 2 separate ones: `CLI_CGICGONE` for -1
and `CLI_CGICERR` for other non-zero exit stati. What state for `waitpid == 0` ?

So, should we de-couple the child-kill-handling completely from the other
CGI request processing? I think so! When do we do this?

- If a CGI request was handled normally. That means if
  `Response::cgiProcessBody` is being called in parallel we should manage the
  kill-handling of the child process. From this point on the exit status does
  not matter anymore. Or, we eval child's state once before we `cgiProcessBody`
  and if everything is good, or child is still running we go on. If this check
  returns non-zero exit we will have to switch the response to 504 or 502.

- If a client just disconnects (clarify the events we will receive in that
  case?!?!) we have to kill-handle but there is no response to be send back to
  client, bc it disconnected.

- ... 

in short: as long as we leave at least the parents read end monitored by epoll,
there should be killing handled. only after ensured kill of child this will be
removed from epoll interest list. So, how do we keep track of this? It wouldn't
be performance-wise smart to check through all clients ... so we need a
`std::map<int, *Client> _cgiKillList` where we add a client that is scheduled
for cgi-killing and in each epoll-cycle we have to check trough that map and
possibly escalate the kill-level from SIGTERM to SIGKILL. Does this client then
even need a state? Well, no. The only we need here is the child's PID!!!! We
don't need the client at all for handling the proper killing of the child
process! And the whole CGI procedure does not have to care about what happens to
the child if it is ready for being killed!


