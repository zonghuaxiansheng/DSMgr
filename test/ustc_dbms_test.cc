#include "buffer_manager.h"
// #include "data_storage_manager.h"
#include <iostream>

enum OPS_E {R, W};
struct OPS {
  OPS_E ops_e_;
  int page_id_;
};

bool runBMgrTraceTest(ustc_dbms::BMgr& bmgr,
                      std::string& trace_path) {
  std::fstream trace;
  trace.open(trace_path, std::ios::in);
  if (!trace.is_open()) {
    std::cerr << "Test: " << __FUNC__
              << " Read trace file failed !"
              << std::endl;
    return false;
  }
  std::vector<OPS> ops_vec;
  int ops_i;
  char buf;
  while (!trace.eof()) {
    OPS ops;
    trace >> ops_i >> buf >> ops.page_id_;
    ops.ops_e_ = (OPS_E)ops_i;
    ops_vec.push_back(ops);
  }
  trace.close();
  int max_page_id = 0;
  for (auto ops : ops_vec) {
    // std::cout << "* Op: " << ops.ops_e_
    //           << " Page id: " << ops.page_id_ << std::endl;
    if (ops.page_id_ > max_page_id) {
      max_page_id = ops.page_id_;
    }
  }
  std::cout << "Test: " << __FUNC__
            << " Max page id is " << max_page_id
            << std::endl;
  // Initial BMgr
  bmgr.InitBMgrTest(max_page_id);
  // Run test
  for (auto ops : ops_vec) {
    switch (ops.ops_e_) {
      case R :
        bmgr.FixPage(ops.page_id_, 0);
        break;
      case W :
        std::cout << "" << std::endl;
    }
  }
}

int main(void) {

  int buffer_size = 128;
  int bucket_size = 16;
  std::string db_path("out/ustc.db");
  std::string trace_path("data/data-5w-50w-zipf.txt");

  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  ustc_dbms::BMgr bmgr(buffer_size,
                       bucket_size,
                       db_path);
  // Run BMgr trace test
  runBMgrTraceTest(bmgr, trace_path);
  // bmgr.InitBMgrTest(1024);
  // for (int i = 1; i < 100; i ++) {
  //   bmgr.FixPage(i, 0);
  // }
  // bmgr.SetDirty(10);
  // bmgr.SetDirty(20);
  // bmgr.PrintFrame(10);
  // bmgr.SetClean(10);
  // bmgr.PrintFrame(10);
  // bmgr.WriteDirtys();
  // auto [page_id, frame_id] = bmgr.FixNewPage();
  // std::cout << "Test: " << __FUNC__
  //           << " FixNewPage return with page_id[" << page_id
  //           << "] frame_id[" << frame_id << "]"
  //           << std::endl;
  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  
  return 0;
}
