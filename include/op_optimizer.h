#ifndef _USTC_OP_OPTIMIZER_H_
#define _USTC_OP_OPTIMIZER_H_

typedef unsigned int dbms_addr_t;

namespace ustc_dbms {

/*!
 * \brief the context of the database operator.
 */
template<OpType>
struct OpCentext {
  /* \brief the operator's name */
  std::string op_name_;
  /* \brief the operator's addr */
  dbms_addr_t op_addr_;
  /* \brief the operator's read/write data length */
  int length_;
};

template<OpType>
class OpOptimizer {
  public:
    OpOptimizer();
  private:
    OpCentext<OpType>* op_ctx_;
};

}   // namespace ustc_dbms

#endif