#include <BRSRtmpMsgArray.h>


namespace BRS 
{
BrsMessageArray::BrsMessageArray(int argMax)
{
      assert(argMax > 0 );
    
      msgs = new BrsSharedPtrMessage*[argMax];
      max = argMax;
      
      zero(argMax);
}

BrsMessageArray::~BrsMessageArray()
{
     SafeDeleteArray(msgs);
}

void BrsMessageArray::free(int count)
{
      for(int i=0;i<count;i++)
      {
	BrsSharedPtrMessage *msg = msgs[i];
	SafeDelete(msg);
	msgs[i] = NULL;
      }
}

void BrsMessageArray::zero(int count)
{
      for(int i =0;i<count;i++)
      {
	msgs[i] = NULL;
      }
}

  
}
