
#ifndef LUPPP_EVENT_HANDLER_DSP_H
#define LUPPP_EVENT_HANDLER_DSP_H


// Library
#include <cstring>
#include <iostream>
#include <jack/ringbuffer.h>

// Internal
#include "jack.hxx"
#include "event.hxx"
#include "controller/controller.hxx"
#include "eventhandler.hxx"

#include "logic.hxx"
#include "state/state.hxx"
#include "timemanager.hxx"
#include "controllerupdater.hxx"

using namespace std;

extern Jack* jack;

void handleDspEvents()
{
  uint availableRead = jack_ringbuffer_read_space( rbToDsp );
  
  while ( availableRead >= sizeof(EventBase) )
  {
    jack_ringbuffer_peek( rbToDsp, (char*)processDspMem, sizeof(EventBase) );
    
    EventBase* e = (EventBase*)processDspMem;
    
    // recheck the size against the actual event size
    if ( availableRead >= e->size() )
    {
      
      // MIDI binding creation: sample the Event.
      if( jack->bindingEventRecordEnable )
      {
        //printf("event %i\n", e->type() );
        jack->bindingEventType = e->type();
        
        const char* target = e->name();
        if ( target )
        {
          EventControllerBindingTarget event( target );
          writeToGuiRingbuffer( &event );
        }
      }
      
      switch ( e->type() )
      {
        case Event::QUIT: {
          if ( availableRead >= sizeof(EventQuit) ) {
            EventQuit ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventQuit) );
            jack->quit();
            // we want to *quit* *fast*: remaining events don't matter!
            return;
            
          } break; }
        
        // ========= SAVE =====
        case Event::STATE_SAVE: {
          if ( availableRead >= sizeof(EventStateSave) ) {
            EventStateSave ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventStateSave) );
            jack->getState()->save();
          } break; }
        case Event::STATE_RESET: {
          if ( availableRead >= sizeof(EventStateReset) ) {
            EventStateReset ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventStateReset) );
            jack->getState()->reset();
          } break; }
        
        // ========= MASTER ===
        case Event::MASTER_VOL: {
          if ( availableRead >= sizeof(EventMasterVol) ) {
            EventMasterVol ev(0);
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventMasterVol) );
            jack->masterVolume( ev.vol );
          } break; }
        case Event::MASTER_RETURN: {
          if ( availableRead >= sizeof(EventMasterReturn) ) {
            EventMasterReturn ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventMasterReturn) );
            if ( ev.ret == RETURN_MAIN ) {
              jack->returnVolume( ev.value );
            }
          } break; }
        case Event::MASTER_INPUT_VOL: {
          if ( availableRead >= sizeof(EventMasterVol) ) {
            EventMasterVol ev(0);
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventMasterVol) );
            jack->getLogic()->masterInputVol( ev.vol );
          } break; }
        case Event::MASTER_INPUT_TO: {
          if ( availableRead >= sizeof(EventMasterInputTo) ) {
            EventMasterInputTo ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventMasterInputTo) );
            jack->getLogic()->masterInputTo( ev.place, ev.value );
          } break; }
        case Event::MASTER_INPUT_TO_ACTIVE: {
          if ( availableRead >= sizeof(EventMasterInputToActive) ) {
            EventMasterInputToActive ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventMasterInputToActive) );
            jack->getLogic()->masterInputToActive( ev.place, ev.active );
          } break; }
        
        // ========= GRID =====
        case Event::GRID_EVENT: {
          if ( availableRead >= sizeof(EventGridEvent) ) {
            EventGridEvent ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventGridEvent) );
            jack->bindingTrack = ev.track;
            jack->bindingScene = ev.scene;
            if ( ev.pressed )
              jack->getGridLogic()->pressed( ev.track, ev.scene );
            else
              jack->getGridLogic()->released( ev.track, ev.scene );
          } break; }
        
        case Event::GRID_LAUNCH_SCENE: {
            EventGridLaunchScene ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventGridLaunchScene) );
            jack->getGridLogic()->launchScene( ev.scene );
            jack->bindingScene = ev.scene;
            break; }
        
        case Event::GRID_SELECT_CLIP_ENABLE: {
            EventGridSelectClipEnable ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventGridSelectClipEnable) );
            jack->getGridLogic()->setSelectTrackScene( ev.enable );
            break; }
        case Event::GRID_SELECT_CLIP_EVENT: {
            EventGridSelectClipEvent ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventGridSelectClipEvent) );
            jack->getGridLogic()->selectedTrackSceneEvent( ev.pressed );
            break; }
        
        case Event::LOOPER_LOAD: {
          if ( availableRead >= sizeof(EventLooperLoad) ) {
            EventLooperLoad ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventLooperLoad) );
            jack->getGridLogic()->load( ev.track, ev.clip, (AudioBuffer*)ev.audioBuffer );
          } break; }
        case Event::METRONOME_ACTIVE: {
          if ( availableRead >= sizeof(EventMetronomeActive) ) {
            EventMetronomeActive ev(false);
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventMetronomeActive) );
            jack->getLogic()->metronomeEnable(ev.active);
          } break; }
        case Event::LOOPER_STATE: {
          if ( availableRead >= sizeof(EventLooperState) ) {
            EventLooperState ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventLooperState) );
            //jack->setLooperState( ev.track, ev.scene, ev.state );
          } break; }
        case Event::LOOPER_LOOP_LENGTH: {
          if ( availableRead >= sizeof(EventLooperLoopLength) ) {
            EventLooperLoopLength ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventLooperLoopLength) );
            jack->getLogic()->looperClipLenght( ev.track, ev.scene, ev.beats );
          } break; }
          
          
        case Event::LOOPER_LOOP_USE_AS_TEMPO: {
          if ( availableRead >= sizeof(EventLooperUseAsTempo) ) {
            EventLooperUseAsTempo ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventLooperUseAsTempo) );
            jack->getLogic()->looperUseAsTempo( ev.track, ev.scene );
          } break; }
          
          
          
        case Event::TIME_BPM: {
          if ( availableRead >= sizeof(EventTimeBPM) ) {
            EventTimeBPM ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventTimeBPM) );
            jack->getTimeManager()->setBpm(ev.bpm);
          } break; }
        case Event::TIME_TEMPO_TAP: {
          if ( availableRead >= sizeof(EventTimeTempoTap) ) {
            EventTimeTempoTap ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventTimeTempoTap) );
            jack->getTimeManager()->tap();
          } break; }
        
        // ======== FX ===========
        
        case Event::FX_REVERB: break;
        /*{
          if ( availableRead >= sizeof(EventFxReverb) ) {
            EventFxReverb ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventFxReverb) );
            // TODO implement reverb
            break; }
          }
        */
        case Event::TRACK_VOLUME: {
          if ( availableRead >= sizeof(EventTrackVol) ) {
            EventTrackVol ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventTrackVol) );
            jack->getLogic()->trackVolume( ev.track, ev.vol );
            jack->bindingTrack = ev.track;
            break; }
          }
        
        case Event::TRACK_RECORD_ARM: {
          if ( availableRead >= sizeof(EventTrackRecordArm) ) {
            EventTrackRecordArm ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventTrackRecordArm) );
            jack->getLogic()->trackRecordArm( ev.track, ev.recordArm );
            jack->bindingTrack  = ev.track;
            jack->bindingActive = ev.recordArm;
            break; }
          }
          
        case Event::TRACK_SEND_ACTIVE: {
          if ( availableRead >= sizeof(EventTrackSendActive) ) {
            EventTrackSendActive ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventTrackSendActive) );
            jack->getLogic()->trackSendActive( ev.track, ev.send, ev.active );
            jack->bindingTrack  = ev.track;
            jack->bindingActive = ev.active;
          } break; }
        
        case Event::TRACK_SEND: {
          if ( availableRead >= sizeof(EventTrackSend) ) {
            EventTrackSend ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventTrackSend) );
            jack->getLogic()->trackSend( ev.track, ev.send, ev.value );
            jack->bindingTrack = ev.track;
            jack->bindingSend  = ev.send;
          } break; }
          
        
        // ========= LUPPP INTERNAL =====
        case Event::LOOPER_REQUEST_BUFFER: {
          if ( availableRead >= sizeof(EventLooperClipRequestBuffer) ) {
            EventLooperClipRequestBuffer ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventLooperClipRequestBuffer) );
            jack->getLooper( ev.track )->setRequestedBuffer( ev.scene, ev.ab );
          } break; }
        
        case Event::REQUEST_SAVE_BUFFER: {
          if ( availableRead >= sizeof(EventRequestSaveBuffer) ) {
            EventRequestSaveBuffer ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventRequestSaveBuffer) );
            //cout << "Save buffer sent with t s ab* " << ev.track << " " << ev.scene << " " << ev.ab << endl;
            jack->getLooper( ev.track )->getClip(ev.scene)->recieveSaveBuffer( ev.ab );
          } break; }
        
        case Event::CONTROLLER_INSTANCE: {
          if ( availableRead >= sizeof(EventControllerInstance) ) {
            EventControllerInstance ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventControllerInstance) );
            jack->getControllerUpdater()->registerController( static_cast<Controller*>(ev.controller) );
          } break; }
        
        case Event::CONTROLLER_BINDING_ENABLE: {
          if ( availableRead >= sizeof(EventControllerBindingEnable) ) {
            EventControllerBindingEnable ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventControllerBindingEnable) );
            jack->bindingEventRecordEnable = ev.enable;
          } break; }
        
        case Event::CONTROLLER_INSTANCE_GET_TO_WRITE: {
          if ( availableRead >= sizeof(EventControllerInstanceGetToWrite) ) {
            EventControllerInstanceGetToWrite ev;
            jack_ringbuffer_read( rbToDsp, (char*)&ev, sizeof(EventControllerInstanceGetToWrite) );
            // get the corresponding Controller with ID, and return it
            Controller* c = jack->getControllerUpdater()->getController( ev.ID );
            EventControllerInstanceGetToWrite tmp( ev.ID, c );
            writeToGuiRingbuffer( &tmp );
          } break; }
        
        default:
          {
            cout << "DSP: Unkown message!! Will clog ringbuffer" << endl;
            // just do nothing
            break;
          }
      }
    }
    else
    {
      // next call will get the half-written event
      return;
    }
    
    // update available read, and loop over events
    availableRead = jack_ringbuffer_read_space( rbToDsp );
  }
}

void writeToDspRingbuffer(EventBase* e)
{
  if ( jack_ringbuffer_write_space(rbToDsp) >= e->size() )
  {
    jack_ringbuffer_write( rbToDsp, (const char*)e, e->size() );
  }
  else
  {
    cout << "->DSP ringbuffer full!" << endl;
  }
}

#endif // LUPPP_EVENT_HANDLER_DSP_H

