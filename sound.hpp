#ifndef SOUND_HPP
#define SOUND_HPP

#include <iostream>
#include <vector> 

#ifdef __GNUC__

#include <cstdint>
#include <cstddef>
#define __PACKED__ __attribute__((packed))

#else

#define STATIC_ASSERT(B) STATIC_ASSERT_IMPL<B>()
template<bool B>
inline void STATIC_ASSERT_IMPL(){
  char STATIC_ASSERT_FAILURE[B] = { 0 };
}

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

inline void my_static_assert(){
  STATIC_ASSERT(sizeof(uint8_t)  == 1);
  STATIC_ASSERT(sizeof(uint16_t) == 2);
  STATIC_ASSERT(sizeof(uint32_t) == 4);
}

#define __PACKED__


#endif // __GNUC__

#ifndef __GNUC__
#pragma pack(1)
#endif

struct MidiHeaderChunk{
  uint32_t chunkID;
  uint32_t chunkSize;
  uint16_t formatType;
  uint16_t numberOfTracks;
  uint16_t timeDivision;
}__PACKED__;

struct MidiFile{
  MidiHeaderChunk header;
  uint8_t data[0];
}__PACKED__;

struct MidiTrackChunk{
  uint32_t chunkID;
  uint32_t chunkSize;
  uint8_t  data[0];
}__PACKED__;

#ifndef __GNUC__
#pragma pack()
#endif

// Check the header is valid or not
bool isValidMidiHeader(const MidiHeaderChunk &header);
// Parse delta time
std::pair<uint32_t, uint32_t> deltaTime(const uint8_t *src);

// Note on only
class MidiEvent{
public:
  uint32_t time;
  uint8_t  note;
  uint8_t  channel;

  MidiEvent(uint32_t time, uint8_t note, uint8_t channel);
};

class Track{
public:
  Track(uint8_t *data);
  std::vector<MidiEvent> events;
};

class Midi{
public:
  Midi(const char *filename);
  ~Midi();

  operator bool () const;

  int getFormat() const;
  int getTrackNum() const ;
  int getTimeDivision() const;

  bool valid() const;

  // for debuging
  void showInfo() const;

  std::vector<Track> tracks;
private:
  int format;
  int trackNum;
  int timeDivision;

  bool _valid;
};

#endif // SOUND_HPP
