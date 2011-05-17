#include <iostream>
#include <algorithm>
#include <vector>
#include <set>

#include <cassert>

#include "sound.hpp"

#define MYNAME "midisearch"
#define VERSION "0.01"

/*
int test(){
  using std::cout;
  using std::endl;

  uint32_t data = 0x0381;
  assert(deltaTime(reinterpret_cast<uint8_t *>(&data)).first  == 0x83);
  assert(deltaTime(reinterpret_cast<uint8_t *>(&data)).second == 2);

  Midi midi("2.MID");
  assert(static_cast<bool>(midi));

  midi.showInfo();

  return 0;
}
*/

typedef std::vector<std::pair<uint32_t, uint8_t> > MidiSummary;

MidiSummary midiSummary(const Midi &sMidi, int ch = -1){
  MidiSummary ret;
  
  for(std::vector<Track>::const_iterator it = sMidi.tracks.begin(); it != sMidi.tracks.end(); ++it){
    for(std::vector<MidiEvent>::const_iterator it2 = it->events.begin(); it2 != it->events.end(); ++it2){
      if(ch == -1 || ch == static_cast<int>(it2->channel))
	ret.push_back(std::make_pair(it2->time, it2->note));
    }
  }
  
  sort(ret.begin(), ret.end());
  return ret;
}

bool includes(const MidiSummary &search,
	      const MidiSummary &searched){
  if(search.size() > searched.size()) return false;

  for(int i = 0; i < searched.size() - search.size(); i++){
    int diff = search[0].second - searched[i].second;
    int tmp  = 0;
    int j = 0;

    while(i+j+tmp+1 < searched.size() && searched[i+j+tmp].first == searched[i+j+tmp+1].first) tmp++;

    for(j = 1; j < search.size() && i + j + tmp < searched.size(); j++){
      while(true){
	if(search[j].second - searched[i+j+tmp].second == diff){
	  break;
	}else{
	  if(i+j+tmp+1 < searched.size() && searched[i+j+tmp].first == searched[i+j+tmp+1].first)
	    tmp++;
	  else
	    break;
	}
      }
      if(search[j].second - searched[i+j+tmp].second != diff) break;
      while(i+j+tmp+1 < searched.size() && searched[i+j+tmp].first == searched[i+j+tmp+1].first) tmp++;
    }

    if(j == search.size()) return true;
  }
  return false;
}

std::set<int> channels(const Midi &sMidi){
  std::set<int> ret;

  for(std::vector<Track>::const_iterator it = sMidi.tracks.begin(); it != sMidi.tracks.end(); ++it){
    for(std::vector<MidiEvent>::const_iterator it2 = it->events.begin(); it2 != it->events.end(); ++it2){
      ret.insert(static_cast<int>(it2->channel));
    }
  }
  
  return ret;
}


int main(int argc, char *argv[]){
  using std::cout;
  using std::endl;

  if(argc < 3){
    cout << "Usage: " << MYNAME << " searchfile files ..." << endl;
    return 0;
  }

  Midi midi(argv[1]);
  if(!midi){
    cout << "An error occurred in loading \"" << argv[1] << "\"" << endl;
    return 0;
  }
  
  MidiSummary search(midiSummary(midi));
  
  for(int i = 2; i < argc; i++){
    Midi sMidi(argv[i]);
    
    if(!sMidi){
      cout << "An error occurred in loading \"" << argv[i] << "\"" << endl;
      continue;
    }
    
    std::set<int> chan = channels(sMidi);
    
    for(std::set<int>::iterator it = chan.begin(); it != chan.end(); ++it){
      MidiSummary searched(midiSummary(sMidi, *it));
      
      if(includes(search, searched)){
	cout << argv[i] << " is matched! (at Channel " << *it << ")" <<  endl;
	break;
      }
    }
  }

  return 0;
}
