#pragma once
#include "ParaNdis-VirtQueue.h"

class CParaNdisAbstractPath
{
public:
#if NDIS_SUPPORT_NDIS620
    CParaNdisAbstractPath()
    {
        memset(&DPCAffinity, 0, sizeof(DPCAffinity));
    }
#else
    CParaNdisAbstractPath() : DPCTargetProcessor(0) {}
#endif

    bool WasInterruptReported() 
    {
        return m_interruptReported;
    }

    void ClearInterruptReport()
    {
        m_interruptReported = false;
    }

    void ReportInterrupt() {
        m_interruptReported = true;
    }

    UINT getMessageIndex() {
        return m_messageIndex;
    }

    UINT getQueueIndex() {
        return m_queueIndex;
    }

    virtual NDIS_STATUS SetupMessageIndex(u16 vector);

    /* TODO - Path classes should inherit from CVirtQueue*/
    virtual void DisableInterrupts()
    {
        m_pVirtQueue->DisableInterrupts();
    }

    void EnableInterrupts()
    {
        m_pVirtQueue->EnableInterrupts();
    }

    void Renew()
    {
        m_pVirtQueue->Renew();
    }

    ULONG getCPUIndex();

#if NDIS_SUPPORT_NDIS620
    GROUP_AFFINITY DPCAffinity;
#else
    KAFFINITY DPCTargetProcessor = 0;
#endif

protected:
    PPARANDIS_ADAPTER m_Context;
    CVirtQueue *m_pVirtQueue;

    u16 m_messageIndex = (u16)-1;
    u16 m_queueIndex = (u16)-1;
    bool m_interruptReported;
};


template <class VQ> class CParaNdisTemplatePath : public CParaNdisAbstractPath {
public:
    CParaNdisTemplatePath() {
        m_pVirtQueue = &m_VirtQueue;
    }


    void Shutdown()
    {
        TSpinLocker LockedContext(m_Lock);
        m_VirtQueue.Shutdown();
    }

    static BOOLEAN _Function_class_(MINIPORT_SYNCHRONIZE_INTERRUPT)
    RestartQueueSynchronously(PVOID ctx)
    {
        auto This = static_cast<CParaNdisTemplatePath<VQ>*>(ctx);
        return !This->m_VirtQueue.Restart();
    }

protected:
    CNdisSpinLock m_Lock;

    VQ m_VirtQueue;
};
