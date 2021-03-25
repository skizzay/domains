ADD_SUBDIRECTORY(skizzay/domains/event_source)
ADD_TEST(event_source_tests bin/event_source_tests)

ADD_SUBDIRECTORY(skizzay/domains/aggregate)
ADD_TEST(event_source_tests bin/aggregate_tests)