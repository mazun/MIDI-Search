template<class T, int n = sizeof(T)>
class reverseByte{
};

template<class T>
class reverseByte<T,1>{
public:
  static T reverse(T val){ return val; }
};
template<class T>
class reverseByte<T,2>{
public:
  static T reverse(T val){ return (val << 8) | (val >> 8); }
};
template<class T>
class reverseByte<T,4>{
public:
  static T reverse(T val){
    val = ((val & 0x00FF00FF) << 8) | ((val >> 8) & 0x00FF00FF);
      return (val << 16) | (val >> 16);
  }
};

template<class T>
inline T reverseOrder(T val){
  return reverseByte<T>::reverse(val);
}
