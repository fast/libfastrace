// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <libfastrace/libfastrace.h>

#include <array>
#include <vector>

int main() {
  std::array<const char*, 10> keys = {"pk1", "pk2", "pk3", "pk4", "ck1",
                                      "ck2", "ck3", "ck4", "ck5", "ck6"};
  std::array<const char*, 10> vals = {"pv1", "pv2", "pv3", "pv4", "cv1",
                                      "cv2", "cv3", "cv4", "cv5", "cv6"};

  fastrace::setConsoleReporter();

  {
    fastrace::SpanContext context;
    fastrace::Span rootSpan("root", context);

    std::vector<std::pair<const char*, const char*>> parentEventProps = {
        {keys[0], vals[0]}, {keys[1], vals[1]}};
    rootSpan.addEvent("parent event", parentEventProps);

    rootSpan.addProperty("phello", "pworld");
    std::vector<std::pair<const char*, const char*>> rootProps = {
        {keys[2], vals[2]}, {keys[3], vals[3]}};
    rootSpan.addProperties(rootProps);

    fastrace::LocalParentGuard guard(rootSpan);
    fastrace::LocalSpan childSpan("child");

    std::vector<std::pair<const char*, const char*>> childEventProps = {
        {keys[4], vals[4]}, {keys[5], vals[5]}};
    childSpan.addEvent("child event", childEventProps);

    childSpan.addProperty("chello", "cworld");
    std::vector<std::pair<const char*, const char*>> childProps1 = {
        {keys[6], vals[6]}, {keys[7], vals[7]}};
    childSpan.addProperties(childProps1);

    childSpan.withProperty("chello2", "cworld2");
    std::vector<std::pair<const char*, const char*>> childProps2 = {
        {keys[8], vals[8]}, {keys[9], vals[9]}};
    childSpan.withProperties(childProps2);
  }

  fastrace::flush();
  return 0;
}
