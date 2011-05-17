#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <sstream>

#include <cstring>

#include "sound.hpp"
#include "reverse.hpp"

std::string toStr(uint8_t val){
  const static char *m[] ={
    "C", "C#",
    "D", "D#",
    "E",
    "F", "F#",
    "G", "G#",
    "A", "A#",
    "B"
  };

  int diff = static_cast<int>(val - 0x3C);
  int height = 3 + diff / 12;
  int note   = (diff + 12 * 12) % 12;
  std::stringstream buff;
  buff << height << m[note];

  return buff.str();
}

std::pair<uint32_t, uint32_t> deltaTime(const uint8_t *src){
  uint32_t ret = 0;
  uint32_t cnt = 0;

  while(true){
    uint8_t now = *src++;

    ret <<= 7;
    ret |= (now & ((1ul << 7) - 1));

    // The size of delta time must not be larger than 4 byte.
    if(++cnt >= 4 || (now & (1ul << 7)) == 0){
      break;
    }
  }

  return std::make_pair(ret, cnt);
}


bool isValidMidiHeader(const MidiHeaderChunk &header){
  const uint32_t ID = 0x4D546864;

  if(reverseOrder(header.chunkID) != ID){
    // std::cerr << "chunkID: " << std::hex << header.chunkID << std::endl;
    return false;
  }

  if(reverseOrder(header.chunkSize) != 0x06){
    // std::cerr << "chunkSize: " << std::hex << header.chunkSize << std::endl;
    return false;
  }

  return true;
}

std::size_t getFileSize(std::ifstream &ifs){
  std::ifstream::pos_type begin, now, end;

  now = ifs.tellg();

  ifs.seekg(0, std::ios_base::end);
  end = ifs.tellg();
  ifs.seekg(0, std::ios_base::beg);
  begin = ifs.tellg();

  ifs.seekg(now);

  return static_cast<size_t>(end - begin);
}

MidiEvent::MidiEvent(uint32_t t, uint8_t n, uint8_t c)
  : time(t), note(n), channel(c){}

/*
 * Implementation of Track class
 */
Track::Track(uint8_t *data){
  MidiTrackChunk *trackChunk = reinterpret_cast<MidiTrackChunk *>(data);
  uint32_t chunkID   = reverseOrder(trackChunk->chunkID);
  uint32_t chunkSize = reverseOrder(trackChunk->chunkSize);
  
  data += 4 + 4;
  uint8_t *dataStart = data;

  uint32_t integratedTime = 0;

  uint8_t  prevState = 0;

  while(dataStart + chunkSize > data){
    std::pair<uint32_t, uint32_t> tmp = deltaTime(data);
    uint32_t time = tmp.first;
    data += tmp.second;

    uint8_t firstByte   = *data++;

    integratedTime += time;

    if(firstByte == 0xF0 || firstByte == 0xF7){
      // SysEx Event
      std::pair<uint32_t, uint32_t> tmp2 = deltaTime(data);
      uint32_t length = tmp2.first;
      data += tmp2.second;

      // Skip message
      data += length;

    }else if(firstByte == 0xFF){
      // Meta Event
      uint8_t eventType   = *data++;
      std::pair<uint32_t, uint32_t> tmp2 = deltaTime(data);
      uint32_t length = tmp2.first;
      data += tmp2.second;

      // Skip message
      data += length;
    }else{
      // Midi Event
      uint8_t nowState;

      if((firstByte & 0x80) == 0){
	// Running state
	nowState = prevState;
      }else{
	prevState = nowState = firstByte;
      }

      uint8_t eventType   = nowState >> 4;
      uint8_t midiChannel = nowState & ((1 << 4) - 1);

      uint8_t param1      = *data++;
      uint8_t param2      = *data++;

      if(eventType == 0xC || eventType == 0xD) data--;

      // note on
      if(eventType == 0x9 && param2 != 0){
	events.push_back(MidiEvent(integratedTime, param1, midiChannel));

	// std::cout << "Note on: " << std::dec << integratedTime << " "
	// << toStr(param1) << " " << static_cast<int>(midiChannel) << std::endl;
      }
    }
  }

  if(dataStart + chunkSize != data)
    throw 11;
}

/*
 * Implementation of Midi class
 */
Midi::~Midi(){
}

bool Midi::valid() const{ return _valid; }

Midi::Midi(const char *filename) : _valid(false){
  MidiFile *data = NULL;

  try{
    // std::cout << filename << std::endl;
#ifndef __GNUC__
    std::locale::global(std::locale("japanese"));
#endif
    std::ifstream ifs(filename, std::ios::binary);
    
    if(!ifs) throw 0;
    
    std::size_t size = getFileSize(ifs);

    data = reinterpret_cast<MidiFile *>(new char[size]);
    
    if(!data) throw 1;
    
    ifs.read(reinterpret_cast<char*>(data),
	     static_cast<std::streamsize>(size));
    
    const MidiHeaderChunk &header = data->header;
    
    if(!isValidMidiHeader(header))
      throw 2;

    this->format       = static_cast<int>(reverseOrder(data->header.formatType));
    this->trackNum     = static_cast<int>(reverseOrder(data->header.numberOfTracks));
    this->timeDivision = static_cast<int>(reverseOrder(data->header.timeDivision));

    // Parse tracks
    uint8_t *ptr   = reinterpret_cast<uint8_t *>(data->data);
    uint8_t *start = reinterpret_cast<uint8_t *>(data);

    while(size > static_cast<std::size_t>(ptr - start)){
      MidiTrackChunk *trackChunk = reinterpret_cast<MidiTrackChunk *>(ptr);
      uint32_t chunkID   = reverseOrder(trackChunk->chunkID);
      uint32_t chunkSize = reverseOrder(trackChunk->chunkSize);


      if(chunkID != 0x4D54726B) throw 4;
      if(ptr + 4 + 4 + chunkSize > start + size) throw 5;

      // std::cout << "### New Track ###" << std::endl;
      tracks.push_back(Track(ptr));

      ptr += 4 + 4 + chunkSize;
    }

    if(start + size != ptr)
      throw 3;

    _valid = true;
  }catch(int e){
    delete [] data;
    // std::cerr << "An error occurred in loading \"" << filename << "\"" << std::endl;
    // std::cerr << "Error code: " << e << std::endl;
  }
}

Midi::operator bool() const{ return _valid; }

int Midi::getFormat() const{ return this->format; }
int Midi::getTrackNum() const{ return this->trackNum; }
int Midi::getTimeDivision() const{ return this->timeDivision; }

void Midi::showInfo() const{
  using std::cout;
  using std::endl;

  cout << "Format type: " << getFormat() << endl;
  cout << "Number of tracks: " << getTrackNum() << endl;
}
