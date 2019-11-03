#include "types.h"

namespace ustc_dbms {
/*
 * \brief Use for buffer data copy.
 */
void dbCopy(char* src_data,
            int src_offset,
            char* dst_data,
            int dst_offset,
            int copy_size) {
  for (int i = 0; i < copy_size; i ++) {
    dst_data[dst_offset + i] = src_data[src_offset + i];
  }
}
/*
 * \brief Convert int to char.
 */
void Int2Char(int in,
              uint8_t* out) {
  for (int i = 3; i >= 0; i --) {
    *(out + i) = (uint8_t)(in >> ((3 - i)*8));
  }
}
/*
 * \brief Convert char to int.
 */
void Char2Int(uint8_t* in,
              int& out) {
  out = 0;
  for (int i = 3; i >= 0; i --) {
    int tmp = (int)*(in + i);
    tmp = (tmp & 0xff) << ((3 - i)*8);
    out |= tmp;
  }
}
}   // namespace ustc_dbms
