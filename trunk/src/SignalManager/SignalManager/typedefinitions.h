#ifndef TYPEDEFINITIONS
#define TYPEDEFINITIONS

enum DspServerState
{
    NotRunning,
    Running,
    InUse
};

namespace ObserverState
{
    enum State
    {
        NotObserving,
        Observing,
        Finished
    };
}
#endif // TYPEDEFINITIONS

