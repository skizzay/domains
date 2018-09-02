add_executable(skizzay_domains_event_sourcing_tests
   ../main.cpp
   unit/hook.t.cpp
   unit/commit_store.t.cpp
   )
target_link_libraries(skizzay_domains_event_sourcing_tests ${CONAN_LIBS})

catch_discover_tests(skizzay_domains_event_sourcing_tests
   TEST_SPEC "[unit][hook]"
   )
catch_discover_tests(skizzay_domains_event_sourcing_tests
   TEST_SPEC "[unit][commit_store]"
   )
catch_discover_tests(skizzay_domains_event_sourcing_tests
   TEST_SPEC "[integration][hook]"
   )
