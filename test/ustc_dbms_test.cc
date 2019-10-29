#include "buffer_manager.h"
#include "data_storage_manager.h"
#include <iostream>

int main(void) {

  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;

  ustc_dbms::BMgr bmgr(1024, 1);
  bmgr.FixPage(111, 0);
  // ...
  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  
  return 0;
}
