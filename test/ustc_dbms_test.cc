#include "buffer_manager.h"
// #include "data_storage_manager.h"
#include <iostream>

enum OPS_E {R, W};
struct OPS {
  OPS_E ops_e_;
  int page_id_;
};

bool RunBMgrTraceTest(ustc_dbms::BMgr& bmgr,
                      std::string& trace_path,
                      bool is_run) {
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
  if (!is_run) {
    max_page_id ++;
    std::cout << "Test: " << __FUNC__
              << " Max page id is " << max_page_id << std::endl;
    // Initial BMgr
    std::cout << "Test: " << __FUNC__
              << " Build db file ..." << std::endl;
    bmgr.InitBMgrTest(max_page_id);
    std::cout << "Test: " << __FUNC__
              << " Build done ..." << std::endl;
  } else {
    std::cout << "Test: " << __FUNC__
              << " Trace test start ..."
              << std::endl;
    // Run test
    for (auto ops : ops_vec) {
      switch (ops.ops_e_) {
        case R :
          bmgr.FixPage(ops.page_id_, 0);
          bmgr.UnfixPage(ops.page_id_);
          break;
        case W :
          bmgr.WritePage(ops.page_id_);
          break;
        default :
          std::cout << "WTF ?" << std::endl;
          break;
      }
    }
    bmgr.WriteDirtys();
    std::cout << "Test: " << __FUNC__
              << " Trace test done ..."
              << std::endl;
    bmgr.PrintIO();
  }
  return true;
}

int main(void) {

  int buffer_size = 1024;
  int bucket_size = 32;
  bool is_build = false;
  std::string db_path("out/data.dbf");
  std::string trace_path("data/data-5w-50w-zipf.txt");
  // std::string trace_path("data/db-data-gen.txt");

  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  ustc_dbms::BMgr bmgr(buffer_size,
                       bucket_size,
                       db_path,
                       is_build);
  // Run BMgr trace test
  RunBMgrTraceTest(bmgr, trace_path, ~is_build);

  // bmgr.InitBMgrTest(1000);
  // for (int i = 1; i < 100; i ++) {
  //   bmgr.FixPage(i, 0);
  // }
  // bmgr.WritePage(20);
  // bmgr.WritePage(30);
  // bmgr.SetDirty(10);
  // bmgr.SetDirty(20);
  // bmgr.PrintFrame(10);
  // bmgr.SetClean(10);
  // bmgr.PrintFrame(10);
  // bmgr.WriteDirtys();
  // bmgr.PrintIO();
  // auto [page_id, frame_id] = bmgr.FixNewPage();
  // std::cout << "Test: " << __FUNC__
  //           << " FixNewPage return with page_id[" << page_id
  //           << "] frame_id[" << frame_id << "]"
  //           << std::endl;
  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  
  return 0;
}
