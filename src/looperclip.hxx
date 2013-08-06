
#ifndef LUPPP_LOOPER_CLIP_H
#define LUPPP_LOOPER_CLIP_H

#include <stdio.h>
#include "config.hxx"
#include "audiobuffer.hxx"

/** LooperClip
 * Represents each clip that a looper can playback. The core of the audio
 * samples is stored in AudioBuffer objects which are dynamically resized. The
 * base size of a AudioBuffer is 1 second's worth, after which larger buffers
 * will be requested.
 * The transition between AudioBuffer instances is seamless: when the clip is
 * running out, the new one is requested. Upon its arrival the current data is
 * copied, and the old buffer is returned for deallocation.
 * 
 * This system allows for arbitrary length recordings, without huge
 * pre-allocated buffers while it is still quite simple.
 * 
 * Each clip has its properties like length and bars/beats, so the Looper knows
 * to dynamically stretch / process the audio appropriately. Controllers and the
 * UI are updated from this data.
**/
class LooperClip
{
  public:
    LooperClip()
    {
      _loaded = false;
      _playing = false;
      _recording = false;
      
      _buffer = 0;
      
      _playhead = 0;
    }
    
    /// loads a sample: eg from disk
    void load( AudioBuffer* ab )
    {
      _loaded = true;
      
      if ( _buffer )
      {
        // unload old buffer!
        printf("LooperClip: FIXME:TODO de-allocate old sample here!\n");
      }
      
      _buffer = ab;
      
      _playhead = 0;
      
      _playing = true;
    }
    
    /// used to update the size of the buffer for this looperclip. The current
    /// data is copied into the new buffer, then the smaller buffer is sent
    /// for de-allocation
    void setRequestedBuffer( AudioBuffer* ab )
    {
      
    }
    
    void record(int count, float* L, float* R)
    {
      // write "count" samples into current buffer.
      if ( _buffer )
      {
        for(int i = 0; i < count; i++)
          _buffer->getData().at( _recordhead ) = *L++;
      }
      
      if(_buffer->getData().size() - _recordhead < LOOPER_SAMPLES_BEFORE_REQUEST)
      {
        // request bigger buffer for this LooperClip
        
      }
      
    }
    
    
    bool loaded(){return _loaded;}
    bool playing(){return _playing;}
    bool recording(){return _recording;}
    
    float getSample()
    {
      if ( _buffer )
      {
        if ( _playhead >= _buffer->getData().size() || _playhead < 0  )
        {
          _playhead = 0;
          printf("looper resetting playhead\n");
        }
        
        std::vector<float>& v = _buffer->getData();
        float tmp = v.at(_playhead);
        _playhead++;
        
        return tmp;
      }
      
      return 0.f;
    }
    
    float getProgress()
    {
      if ( _buffer )
      {
        return float(_playhead) / _buffer->getData().size();
      }
      return 0.f;
    }
    
    // Set
    //void clipLength(int l){_clipLenght = l;}
  
  private:
    bool _loaded;
    bool _recording;
    bool _playing;
    
    unsigned int _playhead;
    unsigned int _recordhead;
    AudioBuffer* _buffer;
};

#endif // LUPPP_LOOPER_CLIP_H

