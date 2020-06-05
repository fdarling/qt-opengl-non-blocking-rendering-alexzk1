#pragma once

#include <QObject>
#include <QThread>
#include <functional>
#include <memory>
#include <atomic>
#include "cm_ctors.h"

//allows to launch lambda in QThread

using TerminateIfTruePtr = std::shared_ptr<std::atomic<bool>>;

inline TerminateIfTruePtr createSharedAtomic(bool value)
{
    return std::make_shared<std::atomic<bool>>(value);
}


//tests atomic bool for expected, if it is - sets it to !expected and returns true
inline bool testandflip(std::atomic<bool>& var, const bool expected)
{
    bool exp{expected};
    return var.compare_exchange_strong(exp, !expected);
}

class LambdaThread : public QThread
{
    Q_OBJECT
public:
    using Functor = std::function<void(TerminateIfTruePtr)>;

    //not sure if QThread likes copying ...guess not
    NO_COPYMOVE(LambdaThread);
    explicit LambdaThread(Functor todo, QObject *parent = nullptr):
        QThread(parent),
        todo(std::move(todo))
    {
        setTerminationEnabled(false);
    }

    ~LambdaThread() final
    {
        stopLambda();
    }

    void stopLambda() const
    {
        *stopper = true;
    }
protected:
    Functor todo;
    const TerminateIfTruePtr stopper{createSharedAtomic(false)};
    void run() final
    {
        todo(stopper);
    }
};

using LambdaThreadPtr = std::shared_ptr<LambdaThread>;


//using macros so syntax formatting not going crazy in QtCreator
#define EMPTY_FUNC [](){}

//allows just execute lambda in QThread, returns shared_ptr for QThread
//once shared_ptr is destroyed it will wait block until thread ends
//don't forget to call start() on returned ptr
inline LambdaThreadPtr createInQThread(LambdaThread::Functor what, std::function<void()> call_before_delete = EMPTY_FUNC)
{
    return LambdaThreadPtr(new LambdaThread(std::move(what)), [call_before_delete](LambdaThread * p)
    {
        if (p)
        {
            p->requestInterruption();
            p->stopLambda();
            p->wait();
            call_before_delete();
            delete p;
        }
    });
}
//cleaning up trickery
#undef EMPTY_FUNC
