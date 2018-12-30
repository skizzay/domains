add_executable(skizzay_domains_event_sourcing_tests
   main.cpp
   domains/event_sourcing/hook.t.cpp
   domains/event_sourcing/commit_store.t.cpp
   domains/event_sourcing/range_holder.t.cpp
   )
target_link_libraries(skizzay_domains_event_sourcing_tests skizzay_domains CONAN_PKG::kerchow
   CONAN_PKG::catch2)

catch_discover_tests(skizzay_domains_event_sourcing_tests
   TEST_SPEC "[unit][hook]"
   )
catch_discover_tests(skizzay_domains_event_sourcing_tests
   TEST_SPEC "[unit][commit_store]"
   )
catch_discover_tests(skizzay_domains_event_sourcing_tests
   TEST_SPEC "[unit][range_holder]"
   )
catch_discover_tests(skizzay_domains_event_sourcing_tests
   TEST_SPEC "[integration][hook]"
   )
