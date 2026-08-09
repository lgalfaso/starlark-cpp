// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <string>
#include <vector>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::runtime::context;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_function;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_range;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::testing::error_handler;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

TEST(StarlarkFloat, Type) {
  EXPECT_EQ("float", starlark_float(1).type());
}

TEST(StarlarkFloat, Primitve) {
  EXPECT_TRUE(starlark_float(1).primitive());
}

TEST(StarlarkFloat, Str) {
  // Follow Python results as much as possible.
  EXPECT_EQ("0.0", starlark_float(0.0).str());
  EXPECT_EQ("-0.0", starlark_float(-0.0).str());
  EXPECT_EQ("1.234", starlark_float(1.234).str());
  EXPECT_EQ("1.0", starlark_float(1.0).str());
  EXPECT_EQ("10.0", starlark_float(10.0).str());
  EXPECT_EQ("100.0", starlark_float(100.0).str());
  EXPECT_EQ("1000.0", starlark_float(1000.0).str());
  EXPECT_EQ("10000.0", starlark_float(10000.0).str());
  EXPECT_EQ("100000.0", starlark_float(100000.0).str());
  EXPECT_EQ("1000000.0", starlark_float(1000000.0).str());
  EXPECT_EQ("10000000.0", starlark_float(10000000.0).str());
  EXPECT_EQ("100000000.0", starlark_float(100000000.0).str());
  EXPECT_EQ("1000000000.0", starlark_float(1000000000.0).str());
  EXPECT_EQ("10000000000.0", starlark_float(10000000000.0).str());
  EXPECT_EQ("100000000000.0", starlark_float(100000000000.0).str());
  EXPECT_EQ("1000000000000.0", starlark_float(1000000000000.0).str());
  EXPECT_EQ("10000000000000.0", starlark_float(10000000000000.0).str());
  EXPECT_EQ("100000000000000.0", starlark_float(100000000000000.0).str());
  EXPECT_EQ("1000000000000000.0", starlark_float(1000000000000000.0).str());
  EXPECT_EQ("1e+16", starlark_float(10000000000000000.0).str());
  EXPECT_EQ("1e+17", starlark_float(100000000000000000.0).str());
  EXPECT_EQ("1234567890123456.0", starlark_float(1234567890123456.0).str());
  EXPECT_EQ("1.2345678901234568e+16", starlark_float(12345678901234567.0).str());
  EXPECT_EQ("1.0000000000000004e+18", starlark_float(1000000000000000321.0).str());
  EXPECT_EQ("-1.234", starlark_float(-1.234).str());
  EXPECT_EQ("-1.0", starlark_float(-1.0).str());
  EXPECT_EQ("-10.0", starlark_float(-10.0).str());
  EXPECT_EQ("-100.0", starlark_float(-100.0).str());
  EXPECT_EQ("-1000.0", starlark_float(-1000.0).str());
  EXPECT_EQ("-10000.0", starlark_float(-10000.0).str());
  EXPECT_EQ("-100000.0", starlark_float(-100000.0).str());
  EXPECT_EQ("-1000000.0", starlark_float(-1000000.0).str());
  EXPECT_EQ("-10000000.0", starlark_float(-10000000.0).str());
  EXPECT_EQ("-100000000.0", starlark_float(-100000000.0).str());
  EXPECT_EQ("-1000000000.0", starlark_float(-1000000000.0).str());
  EXPECT_EQ("-10000000000.0", starlark_float(-10000000000.0).str());
  EXPECT_EQ("-100000000000.0", starlark_float(-100000000000.0).str());
  EXPECT_EQ("-1000000000000.0", starlark_float(-1000000000000.0).str());
  EXPECT_EQ("-10000000000000.0", starlark_float(-10000000000000.0).str());
  EXPECT_EQ("-100000000000000.0", starlark_float(-100000000000000.0).str());
  EXPECT_EQ("-1000000000000000.0", starlark_float(-1000000000000000.0).str());
  EXPECT_EQ("-1e+16", starlark_float(-10000000000000000.0).str());
  EXPECT_EQ("-1e+17", starlark_float(-100000000000000000.0).str());
  EXPECT_EQ("-1234567890123456.0", starlark_float(-1234567890123456.0).str());
  EXPECT_EQ("-1.2345678901234568e+16", starlark_float(-12345678901234567.0).str());
  EXPECT_EQ("-1.0000000000000004e+18", starlark_float(-1000000000000000321.0).str());
  EXPECT_EQ("nan", starlark_float(std::numeric_limits<double>::quiet_NaN()).str());
  EXPECT_EQ("inf", starlark_float(std::numeric_limits<double>::infinity()).str());
  EXPECT_EQ("-inf", starlark_float(-std::numeric_limits<double>::infinity()).str());
}

TEST(StarlarkFloat, StrTenExponents) {
  /*`python
    for e in range(309):
        print('  EXPECT_EQ("{}", starlark_float(1e+{}).str());'.format(str(float('1e{}'.format(e))), e))
  */    
  EXPECT_EQ("1.0", starlark_float(1e+0).str());
  EXPECT_EQ("10.0", starlark_float(1e+1).str());
  EXPECT_EQ("100.0", starlark_float(1e+2).str());
  EXPECT_EQ("1000.0", starlark_float(1e+3).str());
  EXPECT_EQ("10000.0", starlark_float(1e+4).str());
  EXPECT_EQ("100000.0", starlark_float(1e+5).str());
  EXPECT_EQ("1000000.0", starlark_float(1e+6).str());
  EXPECT_EQ("10000000.0", starlark_float(1e+7).str());
  EXPECT_EQ("100000000.0", starlark_float(1e+8).str());
  EXPECT_EQ("1000000000.0", starlark_float(1e+9).str());
  EXPECT_EQ("10000000000.0", starlark_float(1e+10).str());
  EXPECT_EQ("100000000000.0", starlark_float(1e+11).str());
  EXPECT_EQ("1000000000000.0", starlark_float(1e+12).str());
  EXPECT_EQ("10000000000000.0", starlark_float(1e+13).str());
  EXPECT_EQ("100000000000000.0", starlark_float(1e+14).str());
  EXPECT_EQ("1000000000000000.0", starlark_float(1e+15).str());
  EXPECT_EQ("1e+16", starlark_float(1e+16).str());
  EXPECT_EQ("1e+17", starlark_float(1e+17).str());
  EXPECT_EQ("1e+18", starlark_float(1e+18).str());
  EXPECT_EQ("1e+19", starlark_float(1e+19).str());
  EXPECT_EQ("1e+20", starlark_float(1e+20).str());
  EXPECT_EQ("1e+21", starlark_float(1e+21).str());
  EXPECT_EQ("1e+22", starlark_float(1e+22).str());
  EXPECT_EQ("1e+23", starlark_float(1e+23).str());
  EXPECT_EQ("1e+24", starlark_float(1e+24).str());
  EXPECT_EQ("1e+25", starlark_float(1e+25).str());
  EXPECT_EQ("1e+26", starlark_float(1e+26).str());
  EXPECT_EQ("1e+27", starlark_float(1e+27).str());
  EXPECT_EQ("1e+28", starlark_float(1e+28).str());
  EXPECT_EQ("1e+29", starlark_float(1e+29).str());
  EXPECT_EQ("1e+30", starlark_float(1e+30).str());
  EXPECT_EQ("1e+31", starlark_float(1e+31).str());
  EXPECT_EQ("1e+32", starlark_float(1e+32).str());
  EXPECT_EQ("1e+33", starlark_float(1e+33).str());
  EXPECT_EQ("1e+34", starlark_float(1e+34).str());
  EXPECT_EQ("1e+35", starlark_float(1e+35).str());
  EXPECT_EQ("1e+36", starlark_float(1e+36).str());
  EXPECT_EQ("1e+37", starlark_float(1e+37).str());
  EXPECT_EQ("1e+38", starlark_float(1e+38).str());
  EXPECT_EQ("1e+39", starlark_float(1e+39).str());
  EXPECT_EQ("1e+40", starlark_float(1e+40).str());
  EXPECT_EQ("1e+41", starlark_float(1e+41).str());
  EXPECT_EQ("1e+42", starlark_float(1e+42).str());
  EXPECT_EQ("1e+43", starlark_float(1e+43).str());
  EXPECT_EQ("1e+44", starlark_float(1e+44).str());
  EXPECT_EQ("1e+45", starlark_float(1e+45).str());
  EXPECT_EQ("1e+46", starlark_float(1e+46).str());
  EXPECT_EQ("1e+47", starlark_float(1e+47).str());
  EXPECT_EQ("1e+48", starlark_float(1e+48).str());
  EXPECT_EQ("1e+49", starlark_float(1e+49).str());
  EXPECT_EQ("1e+50", starlark_float(1e+50).str());
  EXPECT_EQ("1e+51", starlark_float(1e+51).str());
  EXPECT_EQ("1e+52", starlark_float(1e+52).str());
  EXPECT_EQ("1e+53", starlark_float(1e+53).str());
  EXPECT_EQ("1e+54", starlark_float(1e+54).str());
  EXPECT_EQ("1e+55", starlark_float(1e+55).str());
  EXPECT_EQ("1e+56", starlark_float(1e+56).str());
  EXPECT_EQ("1e+57", starlark_float(1e+57).str());
  EXPECT_EQ("1e+58", starlark_float(1e+58).str());
  EXPECT_EQ("1e+59", starlark_float(1e+59).str());
  EXPECT_EQ("1e+60", starlark_float(1e+60).str());
  EXPECT_EQ("1e+61", starlark_float(1e+61).str());
  EXPECT_EQ("1e+62", starlark_float(1e+62).str());
  EXPECT_EQ("1e+63", starlark_float(1e+63).str());
  EXPECT_EQ("1e+64", starlark_float(1e+64).str());
  EXPECT_EQ("1e+65", starlark_float(1e+65).str());
  EXPECT_EQ("1e+66", starlark_float(1e+66).str());
  EXPECT_EQ("1e+67", starlark_float(1e+67).str());
  EXPECT_EQ("1e+68", starlark_float(1e+68).str());
  EXPECT_EQ("1e+69", starlark_float(1e+69).str());
  EXPECT_EQ("1e+70", starlark_float(1e+70).str());
  EXPECT_EQ("1e+71", starlark_float(1e+71).str());
  EXPECT_EQ("1e+72", starlark_float(1e+72).str());
  EXPECT_EQ("1e+73", starlark_float(1e+73).str());
  EXPECT_EQ("1e+74", starlark_float(1e+74).str());
  EXPECT_EQ("1e+75", starlark_float(1e+75).str());
  EXPECT_EQ("1e+76", starlark_float(1e+76).str());
  EXPECT_EQ("1e+77", starlark_float(1e+77).str());
  EXPECT_EQ("1e+78", starlark_float(1e+78).str());
  EXPECT_EQ("1e+79", starlark_float(1e+79).str());
  EXPECT_EQ("1e+80", starlark_float(1e+80).str());
  EXPECT_EQ("1e+81", starlark_float(1e+81).str());
  EXPECT_EQ("1e+82", starlark_float(1e+82).str());
  EXPECT_EQ("1e+83", starlark_float(1e+83).str());
  EXPECT_EQ("1e+84", starlark_float(1e+84).str());
  EXPECT_EQ("1e+85", starlark_float(1e+85).str());
  EXPECT_EQ("1e+86", starlark_float(1e+86).str());
  EXPECT_EQ("1e+87", starlark_float(1e+87).str());
  EXPECT_EQ("1e+88", starlark_float(1e+88).str());
  EXPECT_EQ("1e+89", starlark_float(1e+89).str());
  EXPECT_EQ("1e+90", starlark_float(1e+90).str());
  EXPECT_EQ("1e+91", starlark_float(1e+91).str());
  EXPECT_EQ("1e+92", starlark_float(1e+92).str());
  EXPECT_EQ("1e+93", starlark_float(1e+93).str());
  EXPECT_EQ("1e+94", starlark_float(1e+94).str());
  EXPECT_EQ("1e+95", starlark_float(1e+95).str());
  EXPECT_EQ("1e+96", starlark_float(1e+96).str());
  EXPECT_EQ("1e+97", starlark_float(1e+97).str());
  EXPECT_EQ("1e+98", starlark_float(1e+98).str());
  EXPECT_EQ("1e+99", starlark_float(1e+99).str());
  EXPECT_EQ("1e+100", starlark_float(1e+100).str());
  EXPECT_EQ("1e+101", starlark_float(1e+101).str());
  EXPECT_EQ("1e+102", starlark_float(1e+102).str());
  EXPECT_EQ("1e+103", starlark_float(1e+103).str());
  EXPECT_EQ("1e+104", starlark_float(1e+104).str());
  EXPECT_EQ("1e+105", starlark_float(1e+105).str());
  EXPECT_EQ("1e+106", starlark_float(1e+106).str());
  EXPECT_EQ("1e+107", starlark_float(1e+107).str());
  EXPECT_EQ("1e+108", starlark_float(1e+108).str());
  EXPECT_EQ("1e+109", starlark_float(1e+109).str());
  EXPECT_EQ("1e+110", starlark_float(1e+110).str());
  EXPECT_EQ("1e+111", starlark_float(1e+111).str());
  EXPECT_EQ("1e+112", starlark_float(1e+112).str());
  EXPECT_EQ("1e+113", starlark_float(1e+113).str());
  EXPECT_EQ("1e+114", starlark_float(1e+114).str());
  EXPECT_EQ("1e+115", starlark_float(1e+115).str());
  EXPECT_EQ("1e+116", starlark_float(1e+116).str());
  EXPECT_EQ("1e+117", starlark_float(1e+117).str());
  EXPECT_EQ("1e+118", starlark_float(1e+118).str());
  EXPECT_EQ("1e+119", starlark_float(1e+119).str());
  EXPECT_EQ("1e+120", starlark_float(1e+120).str());
  EXPECT_EQ("1e+121", starlark_float(1e+121).str());
  EXPECT_EQ("1e+122", starlark_float(1e+122).str());
  EXPECT_EQ("1e+123", starlark_float(1e+123).str());
  EXPECT_EQ("1e+124", starlark_float(1e+124).str());
  EXPECT_EQ("1e+125", starlark_float(1e+125).str());
  EXPECT_EQ("1e+126", starlark_float(1e+126).str());
  EXPECT_EQ("1e+127", starlark_float(1e+127).str());
  EXPECT_EQ("1e+128", starlark_float(1e+128).str());
  EXPECT_EQ("1e+129", starlark_float(1e+129).str());
  EXPECT_EQ("1e+130", starlark_float(1e+130).str());
  EXPECT_EQ("1e+131", starlark_float(1e+131).str());
  EXPECT_EQ("1e+132", starlark_float(1e+132).str());
  EXPECT_EQ("1e+133", starlark_float(1e+133).str());
  EXPECT_EQ("1e+134", starlark_float(1e+134).str());
  EXPECT_EQ("1e+135", starlark_float(1e+135).str());
  EXPECT_EQ("1e+136", starlark_float(1e+136).str());
  EXPECT_EQ("1e+137", starlark_float(1e+137).str());
  EXPECT_EQ("1e+138", starlark_float(1e+138).str());
  EXPECT_EQ("1e+139", starlark_float(1e+139).str());
  EXPECT_EQ("1e+140", starlark_float(1e+140).str());
  EXPECT_EQ("1e+141", starlark_float(1e+141).str());
  EXPECT_EQ("1e+142", starlark_float(1e+142).str());
  EXPECT_EQ("1e+143", starlark_float(1e+143).str());
  EXPECT_EQ("1e+144", starlark_float(1e+144).str());
  EXPECT_EQ("1e+145", starlark_float(1e+145).str());
  EXPECT_EQ("1e+146", starlark_float(1e+146).str());
  EXPECT_EQ("1e+147", starlark_float(1e+147).str());
  EXPECT_EQ("1e+148", starlark_float(1e+148).str());
  EXPECT_EQ("1e+149", starlark_float(1e+149).str());
  EXPECT_EQ("1e+150", starlark_float(1e+150).str());
  EXPECT_EQ("1e+151", starlark_float(1e+151).str());
  EXPECT_EQ("1e+152", starlark_float(1e+152).str());
  EXPECT_EQ("1e+153", starlark_float(1e+153).str());
  EXPECT_EQ("1e+154", starlark_float(1e+154).str());
  EXPECT_EQ("1e+155", starlark_float(1e+155).str());
  EXPECT_EQ("1e+156", starlark_float(1e+156).str());
  EXPECT_EQ("1e+157", starlark_float(1e+157).str());
  EXPECT_EQ("1e+158", starlark_float(1e+158).str());
  EXPECT_EQ("1e+159", starlark_float(1e+159).str());
  EXPECT_EQ("1e+160", starlark_float(1e+160).str());
  EXPECT_EQ("1e+161", starlark_float(1e+161).str());
  EXPECT_EQ("1e+162", starlark_float(1e+162).str());
  EXPECT_EQ("1e+163", starlark_float(1e+163).str());
  EXPECT_EQ("1e+164", starlark_float(1e+164).str());
  EXPECT_EQ("1e+165", starlark_float(1e+165).str());
  EXPECT_EQ("1e+166", starlark_float(1e+166).str());
  EXPECT_EQ("1e+167", starlark_float(1e+167).str());
  EXPECT_EQ("1e+168", starlark_float(1e+168).str());
  EXPECT_EQ("1e+169", starlark_float(1e+169).str());
  EXPECT_EQ("1e+170", starlark_float(1e+170).str());
  EXPECT_EQ("1e+171", starlark_float(1e+171).str());
  EXPECT_EQ("1e+172", starlark_float(1e+172).str());
  EXPECT_EQ("1e+173", starlark_float(1e+173).str());
  EXPECT_EQ("1e+174", starlark_float(1e+174).str());
  EXPECT_EQ("1e+175", starlark_float(1e+175).str());
  EXPECT_EQ("1e+176", starlark_float(1e+176).str());
  EXPECT_EQ("1e+177", starlark_float(1e+177).str());
  EXPECT_EQ("1e+178", starlark_float(1e+178).str());
  EXPECT_EQ("1e+179", starlark_float(1e+179).str());
  EXPECT_EQ("1e+180", starlark_float(1e+180).str());
  EXPECT_EQ("1e+181", starlark_float(1e+181).str());
  EXPECT_EQ("1e+182", starlark_float(1e+182).str());
  EXPECT_EQ("1e+183", starlark_float(1e+183).str());
  EXPECT_EQ("1e+184", starlark_float(1e+184).str());
  EXPECT_EQ("1e+185", starlark_float(1e+185).str());
  EXPECT_EQ("1e+186", starlark_float(1e+186).str());
  EXPECT_EQ("1e+187", starlark_float(1e+187).str());
  EXPECT_EQ("1e+188", starlark_float(1e+188).str());
  EXPECT_EQ("1e+189", starlark_float(1e+189).str());
  EXPECT_EQ("1e+190", starlark_float(1e+190).str());
  EXPECT_EQ("1e+191", starlark_float(1e+191).str());
  EXPECT_EQ("1e+192", starlark_float(1e+192).str());
  EXPECT_EQ("1e+193", starlark_float(1e+193).str());
  EXPECT_EQ("1e+194", starlark_float(1e+194).str());
  EXPECT_EQ("1e+195", starlark_float(1e+195).str());
  EXPECT_EQ("1e+196", starlark_float(1e+196).str());
  EXPECT_EQ("1e+197", starlark_float(1e+197).str());
  EXPECT_EQ("1e+198", starlark_float(1e+198).str());
  EXPECT_EQ("1e+199", starlark_float(1e+199).str());
  EXPECT_EQ("1e+200", starlark_float(1e+200).str());
  EXPECT_EQ("1e+201", starlark_float(1e+201).str());
  EXPECT_EQ("1e+202", starlark_float(1e+202).str());
  EXPECT_EQ("1e+203", starlark_float(1e+203).str());
  EXPECT_EQ("1e+204", starlark_float(1e+204).str());
  EXPECT_EQ("1e+205", starlark_float(1e+205).str());
  EXPECT_EQ("1e+206", starlark_float(1e+206).str());
  EXPECT_EQ("1e+207", starlark_float(1e+207).str());
  EXPECT_EQ("1e+208", starlark_float(1e+208).str());
  EXPECT_EQ("1e+209", starlark_float(1e+209).str());
  EXPECT_EQ("1e+210", starlark_float(1e+210).str());
  EXPECT_EQ("1e+211", starlark_float(1e+211).str());
  EXPECT_EQ("1e+212", starlark_float(1e+212).str());
  EXPECT_EQ("1e+213", starlark_float(1e+213).str());
  EXPECT_EQ("1e+214", starlark_float(1e+214).str());
  EXPECT_EQ("1e+215", starlark_float(1e+215).str());
  EXPECT_EQ("1e+216", starlark_float(1e+216).str());
  EXPECT_EQ("1e+217", starlark_float(1e+217).str());
  EXPECT_EQ("1e+218", starlark_float(1e+218).str());
  EXPECT_EQ("1e+219", starlark_float(1e+219).str());
  EXPECT_EQ("1e+220", starlark_float(1e+220).str());
  EXPECT_EQ("1e+221", starlark_float(1e+221).str());
  EXPECT_EQ("1e+222", starlark_float(1e+222).str());
  EXPECT_EQ("1e+223", starlark_float(1e+223).str());
  EXPECT_EQ("1e+224", starlark_float(1e+224).str());
  EXPECT_EQ("1e+225", starlark_float(1e+225).str());
  EXPECT_EQ("1e+226", starlark_float(1e+226).str());
  EXPECT_EQ("1e+227", starlark_float(1e+227).str());
  EXPECT_EQ("1e+228", starlark_float(1e+228).str());
  EXPECT_EQ("1e+229", starlark_float(1e+229).str());
  EXPECT_EQ("1e+230", starlark_float(1e+230).str());
  EXPECT_EQ("1e+231", starlark_float(1e+231).str());
  EXPECT_EQ("1e+232", starlark_float(1e+232).str());
  EXPECT_EQ("1e+233", starlark_float(1e+233).str());
  EXPECT_EQ("1e+234", starlark_float(1e+234).str());
  EXPECT_EQ("1e+235", starlark_float(1e+235).str());
  EXPECT_EQ("1e+236", starlark_float(1e+236).str());
  EXPECT_EQ("1e+237", starlark_float(1e+237).str());
  EXPECT_EQ("1e+238", starlark_float(1e+238).str());
  EXPECT_EQ("1e+239", starlark_float(1e+239).str());
  EXPECT_EQ("1e+240", starlark_float(1e+240).str());
  EXPECT_EQ("1e+241", starlark_float(1e+241).str());
  EXPECT_EQ("1e+242", starlark_float(1e+242).str());
  EXPECT_EQ("1e+243", starlark_float(1e+243).str());
  EXPECT_EQ("1e+244", starlark_float(1e+244).str());
  EXPECT_EQ("1e+245", starlark_float(1e+245).str());
  EXPECT_EQ("1e+246", starlark_float(1e+246).str());
  EXPECT_EQ("1e+247", starlark_float(1e+247).str());
  EXPECT_EQ("1e+248", starlark_float(1e+248).str());
  EXPECT_EQ("1e+249", starlark_float(1e+249).str());
  EXPECT_EQ("1e+250", starlark_float(1e+250).str());
  EXPECT_EQ("1e+251", starlark_float(1e+251).str());
  EXPECT_EQ("1e+252", starlark_float(1e+252).str());
  EXPECT_EQ("1e+253", starlark_float(1e+253).str());
  EXPECT_EQ("1e+254", starlark_float(1e+254).str());
  EXPECT_EQ("1e+255", starlark_float(1e+255).str());
  EXPECT_EQ("1e+256", starlark_float(1e+256).str());
  EXPECT_EQ("1e+257", starlark_float(1e+257).str());
  EXPECT_EQ("1e+258", starlark_float(1e+258).str());
  EXPECT_EQ("1e+259", starlark_float(1e+259).str());
  EXPECT_EQ("1e+260", starlark_float(1e+260).str());
  EXPECT_EQ("1e+261", starlark_float(1e+261).str());
  EXPECT_EQ("1e+262", starlark_float(1e+262).str());
  EXPECT_EQ("1e+263", starlark_float(1e+263).str());
  EXPECT_EQ("1e+264", starlark_float(1e+264).str());
  EXPECT_EQ("1e+265", starlark_float(1e+265).str());
  EXPECT_EQ("1e+266", starlark_float(1e+266).str());
  EXPECT_EQ("1e+267", starlark_float(1e+267).str());
  EXPECT_EQ("1e+268", starlark_float(1e+268).str());
  EXPECT_EQ("1e+269", starlark_float(1e+269).str());
  EXPECT_EQ("1e+270", starlark_float(1e+270).str());
  EXPECT_EQ("1e+271", starlark_float(1e+271).str());
  EXPECT_EQ("1e+272", starlark_float(1e+272).str());
  EXPECT_EQ("1e+273", starlark_float(1e+273).str());
  EXPECT_EQ("1e+274", starlark_float(1e+274).str());
  EXPECT_EQ("1e+275", starlark_float(1e+275).str());
  EXPECT_EQ("1e+276", starlark_float(1e+276).str());
  EXPECT_EQ("1e+277", starlark_float(1e+277).str());
  EXPECT_EQ("1e+278", starlark_float(1e+278).str());
  EXPECT_EQ("1e+279", starlark_float(1e+279).str());
  EXPECT_EQ("1e+280", starlark_float(1e+280).str());
  EXPECT_EQ("1e+281", starlark_float(1e+281).str());
  EXPECT_EQ("1e+282", starlark_float(1e+282).str());
  EXPECT_EQ("1e+283", starlark_float(1e+283).str());
  EXPECT_EQ("1e+284", starlark_float(1e+284).str());
  EXPECT_EQ("1e+285", starlark_float(1e+285).str());
  EXPECT_EQ("1e+286", starlark_float(1e+286).str());
  EXPECT_EQ("1e+287", starlark_float(1e+287).str());
  EXPECT_EQ("1e+288", starlark_float(1e+288).str());
  EXPECT_EQ("1e+289", starlark_float(1e+289).str());
  EXPECT_EQ("1e+290", starlark_float(1e+290).str());
  EXPECT_EQ("1e+291", starlark_float(1e+291).str());
  EXPECT_EQ("1e+292", starlark_float(1e+292).str());
  EXPECT_EQ("1e+293", starlark_float(1e+293).str());
  EXPECT_EQ("1e+294", starlark_float(1e+294).str());
  EXPECT_EQ("1e+295", starlark_float(1e+295).str());
  EXPECT_EQ("1e+296", starlark_float(1e+296).str());
  EXPECT_EQ("1e+297", starlark_float(1e+297).str());
  EXPECT_EQ("1e+298", starlark_float(1e+298).str());
  EXPECT_EQ("1e+299", starlark_float(1e+299).str());
  EXPECT_EQ("1e+300", starlark_float(1e+300).str());
  EXPECT_EQ("1e+301", starlark_float(1e+301).str());
  EXPECT_EQ("1e+302", starlark_float(1e+302).str());
  EXPECT_EQ("1e+303", starlark_float(1e+303).str());
  EXPECT_EQ("1e+304", starlark_float(1e+304).str());
  EXPECT_EQ("1e+305", starlark_float(1e+305).str());
  EXPECT_EQ("1e+306", starlark_float(1e+306).str());
  EXPECT_EQ("1e+307", starlark_float(1e+307).str());
  EXPECT_EQ("1e+308", starlark_float(1e+308).str());
}

TEST(StarlarkFloat, Truthy) {
  EXPECT_TRUE(starlark_float(-std::numeric_limits<double>::infinity()).truthy());
  EXPECT_TRUE(starlark_float(-1.0).truthy());
  EXPECT_FALSE(starlark_float(-0.0).truthy());
  EXPECT_FALSE(starlark_float(0.0).truthy());
  EXPECT_TRUE(starlark_float(1.0).truthy());
  EXPECT_TRUE(starlark_float(std::numeric_limits<double>::infinity()).truthy());
  EXPECT_TRUE(starlark_float(std::numeric_limits<double>::quiet_NaN()).truthy());
}

TEST(StarlarkFloat, Equals) {
  EXPECT_TRUE(starlark_float(-1).equals(starlark_integer(-1)));
  EXPECT_TRUE(starlark_float(-1).equals(starlark_bigint(-1)));
  EXPECT_TRUE(starlark_float(-1).equals(starlark_float(-1)));

  EXPECT_TRUE(starlark_float(0).equals(starlark_integer(0)));
  EXPECT_TRUE(starlark_float(0).equals(starlark_bigint(0)));
  EXPECT_TRUE(starlark_float(0).equals(starlark_float(0)));

  EXPECT_TRUE(starlark_float(1).equals(starlark_integer(1)));
  EXPECT_TRUE(starlark_float(1).equals(starlark_bigint(1)));
  EXPECT_TRUE(starlark_float(1).equals(starlark_float(1)));

  EXPECT_FALSE(starlark_float(1).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_float(1).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_float(1).equals(starlark_float(0)));

  EXPECT_FALSE(starlark_float(1).equals(starlark_integer(-1)));
  EXPECT_FALSE(starlark_float(1).equals(starlark_bigint(-1)));
  EXPECT_FALSE(starlark_float(1).equals(starlark_float(-1)));

  EXPECT_FALSE(starlark_float(1.1).equals(starlark_integer(1)));
  EXPECT_FALSE(starlark_float(1.1).equals(starlark_bigint(1)));
  EXPECT_TRUE(starlark_float(-0.0).equals(starlark_float(0.0)));
  // Starlark mandates that `NaN == NaN`.
  EXPECT_TRUE(starlark_float(NAN).equals(starlark_float(NAN)));

  EXPECT_FALSE(starlark_float(0).equals(starlark_bool(false)));
}

TEST(StarlarkFloat, EqualsExact) {
  EXPECT_FALSE(starlark_float((1L<<53)+1).equals(starlark_integer((1L<<53)+1)));
  EXPECT_FALSE(starlark_float((1L<<53)+1).equals(starlark_bigint((1L<<53)+1)));
  EXPECT_TRUE(starlark_float(std::numeric_limits<int64_t>::min()).equals(starlark_integer(std::numeric_limits<int64_t>::min())));
  EXPECT_TRUE(starlark_float(std::numeric_limits<int64_t>::min()).equals(starlark_bigint(std::numeric_limits<int64_t>::min())));
  EXPECT_FALSE(starlark_float(std::numeric_limits<int64_t>::max()).equals(starlark_integer(std::numeric_limits<int64_t>::max())));
  EXPECT_FALSE(starlark_float(std::numeric_limits<int64_t>::max()).equals(starlark_bigint(std::numeric_limits<int64_t>::max())));
  EXPECT_FALSE(starlark_float(std::numeric_limits<double>::infinity()).equals(starlark_integer(std::numeric_limits<int64_t>::max())));
  EXPECT_FALSE(starlark_float(std::numeric_limits<double>::infinity()).equals(starlark_bigint(number::one() << 2000)));
}

TEST(StarlarkFloat, EqualsVsBigInt) {
  EXPECT_TRUE(starlark_float(1e50).equals(starlark_bigint(starlark::bigint::parse_number("100000000000000007629769841091887003294964970946560", nullptr, 0))));
}

TEST(StarlarkFloat, Hash) {
  EXPECT_EQ(starlark_float(1e50).hash(), 1387127493139725924);
  EXPECT_EQ(starlark_bigint(starlark::bigint::parse_number("100000000000000007629769841091887003294964970946560", nullptr, 0)).hash(), 1387127493139725924);
}

void cmp_helper(starlark::result::status_or<int> cmp, auto matcher) {
  ASSERT_TRUE(cmp.ok());
  EXPECT_THAT(*cmp, matcher);
}

TEST(StarlarkFloat, OrderVsFloat) {
  error_handler error_callback;

  cmp_helper(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_float(-1e50), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_float(-std::numeric_limits<double>::infinity()), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1e50).cmp(starlark_float(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_float(-1e-50), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1e-50).cmp(starlark_float(0.0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(0.0).cmp(starlark_float(1e-50), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1e-50).cmp(starlark_float(1.0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1.0).cmp(starlark_float(1e50), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1e50).cmp(starlark_float(std::numeric_limits<double>::infinity()), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(std::numeric_limits<double>::infinity()).cmp(starlark_float(NAN), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(NAN).cmp(starlark_float(std::numeric_limits<double>::infinity()), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(NAN).cmp(starlark_float(NAN), "cmp", error_callback), Eq(0));
}

TEST(StarlarkFloat, OrderVsInteger) {
  error_handler error_callback;

  cmp_helper(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_integer(0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(std::numeric_limits<double>::infinity()).cmp(starlark_integer(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(NAN).cmp(starlark_integer(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(0).cmp(starlark_integer(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(0).cmp(starlark_integer(1), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(-1).cmp(starlark_integer(-1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(1), "cmp", error_callback), Eq(0));


  cmp_helper(starlark_float(0).cmp(starlark_integer(0), "cmp", error_callback), Eq(0));

  cmp_helper(starlark_float(-2).cmp(starlark_integer(-2), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-2).cmp(starlark_integer(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-2).cmp(starlark_integer(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-2).cmp(starlark_integer(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(-1).cmp(starlark_integer(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(-1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(1).cmp(starlark_integer(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(2).cmp(starlark_integer(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_integer(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_integer(1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_integer(2), "cmp", error_callback), Eq(0));

  cmp_helper(starlark_float(-1.25).cmp(starlark_integer(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1.25).cmp(starlark_integer(1), "cmp", error_callback), Gt(0));

  cmp_helper(starlark_float((1L << 53) + 1).cmp(starlark_integer((1L << 53) + 1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float((1L << 53) + (1L << 49) + 1).cmp(starlark_integer((1L << 53) + (1L << 50) + 1), "cmp", error_callback), Lt(0));
}

TEST(StarlarkFloat, OrderVsBigInt) {
  error_handler error_callback;

  cmp_helper(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_bigint(0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(std::numeric_limits<double>::infinity()).cmp(starlark_bigint(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(NAN).cmp(starlark_bigint(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(0).cmp(starlark_bigint(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(0).cmp(starlark_bigint(1), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(-1).cmp(starlark_bigint(-1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(1), "cmp", error_callback), Eq(0));


  cmp_helper(starlark_float(0).cmp(starlark_bigint(0), "cmp", error_callback), Eq(0));

  cmp_helper(starlark_float(-2).cmp(starlark_bigint(-2), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-2).cmp(starlark_bigint(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-2).cmp(starlark_bigint(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-2).cmp(starlark_bigint(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(-1).cmp(starlark_bigint(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(-1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(1).cmp(starlark_bigint(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(2).cmp(starlark_bigint(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_bigint(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_bigint(1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_bigint(2), "cmp", error_callback), Eq(0));


  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100100000000000007629769841091887003294964970946560", nullptr, 0)), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000006629769841091887003294964970946560", nullptr, 0)), "cmp", error_callback), Gt(0));

  cmp_helper(starlark_float(-1.25).cmp(starlark_bigint(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1.25).cmp(starlark_bigint(1), "cmp", error_callback), Gt(0));

  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946559", nullptr, 0)), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946560", nullptr, 0)), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946561", nullptr, 0)), "cmp", error_callback), Lt(0));
}

TEST(StarlarkFloat, OrderVsBool) {
  error_handler error_callback;
  starlark_bool obj_true(true);

  auto cmp = starlark_float(1).cmp(obj_true, "<", error_callback);
  ASSERT_FALSE(cmp.ok());
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'float' and 'bool'");
}

TEST(StarlarkFloat, OrderExact) {
  error_handler error_callback;

  cmp_helper(starlark_float((1L<<53)+1).cmp(starlark_integer((1L<<53)+1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float((1L<<53)+1).cmp(starlark_bigint((1L<<53)+1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(std::numeric_limits<int64_t>::min()).cmp(starlark_integer(std::numeric_limits<int64_t>::min()), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(std::numeric_limits<int64_t>::min()).cmp(starlark_bigint(std::numeric_limits<int64_t>::min()), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(std::numeric_limits<int64_t>::max()).cmp(starlark_integer(std::numeric_limits<int64_t>::max()), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(std::numeric_limits<int64_t>::max()).cmp(starlark_bigint(std::numeric_limits<int64_t>::max()), "cmp", error_callback), Gt(0));
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkFloat, PlusEqualsAssign) {
  starlark_float f1(1.0);
  starlark_float f2(2.0);
  starlark_integer i1(3);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.plus_equals_assign(f2, ctx, error_callback);
  auto* result2 = f1.plus_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.plus_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("3.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("4.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("5.0", result3->str());
}

TEST(StarlarkFloat, PlusEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.plus_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for +=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryPlus) {
  starlark_float f1(1.0);
  starlark_float f2(2.0);
  starlark_integer i1(3);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_plus(f2, ctx, error_callback);
  auto* result2 = f1.binary_plus(i1, ctx, error_callback);
  auto* result3 = f1.binary_plus(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("3.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("4.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("5.0", result3->str());
}

TEST(StarlarkFloat, BinaryPlusError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_plus(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for +: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryPlusOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_plus(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, MinusEqualsAssign) {
  starlark_float f1(2.0);
  starlark_float f2(3.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.minus_equals_assign(f2, ctx, error_callback);
  auto* result2 = f1.minus_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.minus_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("-1.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("-2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("-6.0", result3->str());
}

TEST(StarlarkFloat, MinusEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.minus_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for -=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryMinus) {
  starlark_float f1(2.0);
  starlark_float f2(3.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_minus(f2, ctx, error_callback);
  auto* result2 = f1.binary_minus(i1, ctx, error_callback);
  auto* result3 = f1.binary_minus(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("-1.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("-2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("-6.0", result3->str());
}

TEST(StarlarkFloat, BinaryMinusError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_minus(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for -: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryMinusOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_minus(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, StarEqualsAssign) {
  starlark_float f1(2.0);
  starlark_float f2(3.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.star_equals_assign(f2, ctx, error_callback);
  auto* result2 = f1.star_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.star_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("6.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("8.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("16.0", result3->str());
}

TEST(StarlarkFloat, StarEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.star_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for *=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryStar) {
  starlark_float f1(2.0);
  starlark_float f2(3.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_star(f2, ctx, error_callback);
  auto* result2 = f1.binary_star(i1, ctx, error_callback);
  auto* result3 = f1.binary_star(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("6.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("8.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("16.0", result3->str());
}

TEST(StarlarkFloat, BinaryStarError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_star(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for *: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryStarOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_star(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, SlashEqualsAssign) {
  starlark_float f1(10.0);
  starlark_float f2(-10.0);
  starlark_float f3(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.slash_equals_assign(f3, ctx, error_callback);
  auto* result2 = f1.slash_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.slash_equals_assign(b1, ctx, error_callback);
  auto* result4 = f2.slash_equals_assign(f3, ctx, error_callback);
  auto* result5 = f2.slash_equals_assign(i1, ctx, error_callback);
  auto* result6 = f2.slash_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("5.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.5", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("2.5", result3->str());
  ASSERT_NE(result4, nullptr);
  EXPECT_EQ("-5.0", result4->str());
  ASSERT_NE(result5, nullptr);
  EXPECT_EQ("-2.5", result5->str());
  ASSERT_NE(result6, nullptr);
  EXPECT_EQ("-2.5", result6->str());
}

TEST(StarlarkFloat, SlashEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.slash_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for /=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinarySlash) {
  starlark_float f1(10.0);
  starlark_float f2(-10.0);
  starlark_float f3(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_slash(f3, ctx, error_callback);
  auto* result2 = f1.binary_slash(i1, ctx, error_callback);
  auto* result3 = f1.binary_slash(b1, ctx, error_callback);
  auto* result4 = f2.binary_slash(f3, ctx, error_callback);
  auto* result5 = f2.binary_slash(i1, ctx, error_callback);
  auto* result6 = f2.binary_slash(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("5.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.5", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("2.5", result3->str());
  ASSERT_NE(result4, nullptr);
  EXPECT_EQ("-5.0", result4->str());
  ASSERT_NE(result5, nullptr);
  EXPECT_EQ("-2.5", result5->str());
  ASSERT_NE(result6, nullptr);
  EXPECT_EQ("-2.5", result6->str());
}

TEST(StarlarkFloat, BinarySlashError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for /: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinarySlashOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, BinarySlashZeroFloatError) {
  starlark_float f0(0.0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(f0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinarySlashZeroIntError) {
  starlark_integer i0(0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(i0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinarySlashZeroBigintError) {
  starlark_bigint b0(number::zero());
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(b0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, SlashSlashEqualsAssign) {
  starlark_float f1(10.0);
  starlark_float f2(-10.0);
  starlark_float f3(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.slash_slash_equals_assign(f3, ctx, error_callback);
  auto* result2 = f1.slash_slash_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.slash_slash_equals_assign(b1, ctx, error_callback);
  auto* result4 = f2.slash_slash_equals_assign(f3, ctx, error_callback);
  auto* result5 = f2.slash_slash_equals_assign(i1, ctx, error_callback);
  auto* result6 = f2.slash_slash_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("5.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("2.0", result3->str());
  ASSERT_NE(result4, nullptr);
  EXPECT_EQ("-5.0", result4->str());
  ASSERT_NE(result5, nullptr);
  EXPECT_EQ("-3.0", result5->str());
  ASSERT_NE(result6, nullptr);
  EXPECT_EQ("-3.0", result6->str());
}

TEST(StarlarkFloat, SlashSlashEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.slash_slash_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for //=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinarySlashSlash) {
  starlark_float f1(10.0);
  starlark_float f2(-10.0);
  starlark_float f3(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_slash_slash(f3, ctx, error_callback);
  auto* result2 = f1.binary_slash_slash(i1, ctx, error_callback);
  auto* result3 = f1.binary_slash_slash(b1, ctx, error_callback);
  auto* result4 = f2.binary_slash_slash(f3, ctx, error_callback);
  auto* result5 = f2.binary_slash_slash(i1, ctx, error_callback);
  auto* result6 = f2.binary_slash_slash(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("5.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("2.0", result3->str());
  ASSERT_NE(result4, nullptr);
  EXPECT_EQ("-5.0", result4->str());
  ASSERT_NE(result5, nullptr);
  EXPECT_EQ("-3.0", result5->str());
  ASSERT_NE(result6, nullptr);
  EXPECT_EQ("-3.0", result6->str());
}

TEST(StarlarkFloat, BinarySlashSlashError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for //: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinarySlashSlashOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, BinarySlashSlashZeroFloatError) {
  starlark_float f0(0.0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(f0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinarySlashSlashZeroIntError) {
  starlark_integer i0(0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(i0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinarySlashSlashZeroBigintError) {
  starlark_bigint b0(number::zero());
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(b0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinaryPercent) {
  auto test = [](double n, int64_t d, std::string_view r) {
    starlark_float num(n);
    starlark_float denf(d);
    starlark_integer deni(d);
    starlark_bigint denb(d);
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    auto* resultf1 = num.binary_percent(denf, ctx, error_callback);
    auto* resulti1 = num.binary_percent(deni, ctx, error_callback);
    auto* resultb1 = num.binary_percent(denb, ctx, error_callback);
    auto* resultf2 = num.percent_equals_assign(denf, ctx, error_callback);
    auto* resulti2 = num.percent_equals_assign(deni, ctx, error_callback);
    auto* resultb2 = num.percent_equals_assign(denb, ctx, error_callback);

    EXPECT_THAT(error_callback.messages, SizeIs(0));
    EXPECT_NE(resultf1, nullptr);
    EXPECT_NE(resulti1, nullptr);
    EXPECT_NE(resultb1, nullptr);
    EXPECT_EQ(r, resultf1->str());
    EXPECT_EQ(r, resulti1->str());
    EXPECT_EQ(r, resultb1->str());
    EXPECT_NE(resultf2, nullptr);
    EXPECT_NE(resulti2, nullptr);
    EXPECT_NE(resultb2, nullptr);
    EXPECT_EQ(r, resultf2->str());
    EXPECT_EQ(r, resulti2->str());
    EXPECT_EQ(r, resultb2->str());
  };
  /*
  ```python
  for a in range(-10, 11):
      for b in range(-10, 11):
          if b!=0:
              print('  test({}, {}, "{}");'.format(float(a), float(b), (float(a) % float(b))))
  ```
  */

  test(-10.0, -10.0, "-0.0");
  test(-10.0, -9.0, "-1.0");
  test(-10.0, -8.0, "-2.0");
  test(-10.0, -7.0, "-3.0");
  test(-10.0, -6.0, "-4.0");
  test(-10.0, -5.0, "-0.0");
  test(-10.0, -4.0, "-2.0");
  test(-10.0, -3.0, "-1.0");
  test(-10.0, -2.0, "-0.0");
  test(-10.0, -1.0, "-0.0");
  test(-10.0, 1.0, "0.0");
  test(-10.0, 2.0, "0.0");
  test(-10.0, 3.0, "2.0");
  test(-10.0, 4.0, "2.0");
  test(-10.0, 5.0, "0.0");
  test(-10.0, 6.0, "2.0");
  test(-10.0, 7.0, "4.0");
  test(-10.0, 8.0, "6.0");
  test(-10.0, 9.0, "8.0");
  test(-10.0, 10.0, "0.0");
  test(-9.0, -10.0, "-9.0");
  test(-9.0, -9.0, "-0.0");
  test(-9.0, -8.0, "-1.0");
  test(-9.0, -7.0, "-2.0");
  test(-9.0, -6.0, "-3.0");
  test(-9.0, -5.0, "-4.0");
  test(-9.0, -4.0, "-1.0");
  test(-9.0, -3.0, "-0.0");
  test(-9.0, -2.0, "-1.0");
  test(-9.0, -1.0, "-0.0");
  test(-9.0, 1.0, "0.0");
  test(-9.0, 2.0, "1.0");
  test(-9.0, 3.0, "0.0");
  test(-9.0, 4.0, "3.0");
  test(-9.0, 5.0, "1.0");
  test(-9.0, 6.0, "3.0");
  test(-9.0, 7.0, "5.0");
  test(-9.0, 8.0, "7.0");
  test(-9.0, 9.0, "0.0");
  test(-9.0, 10.0, "1.0");
  test(-8.0, -10.0, "-8.0");
  test(-8.0, -9.0, "-8.0");
  test(-8.0, -8.0, "-0.0");
  test(-8.0, -7.0, "-1.0");
  test(-8.0, -6.0, "-2.0");
  test(-8.0, -5.0, "-3.0");
  test(-8.0, -4.0, "-0.0");
  test(-8.0, -3.0, "-2.0");
  test(-8.0, -2.0, "-0.0");
  test(-8.0, -1.0, "-0.0");
  test(-8.0, 1.0, "0.0");
  test(-8.0, 2.0, "0.0");
  test(-8.0, 3.0, "1.0");
  test(-8.0, 4.0, "0.0");
  test(-8.0, 5.0, "2.0");
  test(-8.0, 6.0, "4.0");
  test(-8.0, 7.0, "6.0");
  test(-8.0, 8.0, "0.0");
  test(-8.0, 9.0, "1.0");
  test(-8.0, 10.0, "2.0");
  test(-7.0, -10.0, "-7.0");
  test(-7.0, -9.0, "-7.0");
  test(-7.0, -8.0, "-7.0");
  test(-7.0, -7.0, "-0.0");
  test(-7.0, -6.0, "-1.0");
  test(-7.0, -5.0, "-2.0");
  test(-7.0, -4.0, "-3.0");
  test(-7.0, -3.0, "-1.0");
  test(-7.0, -2.0, "-1.0");
  test(-7.0, -1.0, "-0.0");
  test(-7.0, 1.0, "0.0");
  test(-7.0, 2.0, "1.0");
  test(-7.0, 3.0, "2.0");
  test(-7.0, 4.0, "1.0");
  test(-7.0, 5.0, "3.0");
  test(-7.0, 6.0, "5.0");
  test(-7.0, 7.0, "0.0");
  test(-7.0, 8.0, "1.0");
  test(-7.0, 9.0, "2.0");
  test(-7.0, 10.0, "3.0");
  test(-6.0, -10.0, "-6.0");
  test(-6.0, -9.0, "-6.0");
  test(-6.0, -8.0, "-6.0");
  test(-6.0, -7.0, "-6.0");
  test(-6.0, -6.0, "-0.0");
  test(-6.0, -5.0, "-1.0");
  test(-6.0, -4.0, "-2.0");
  test(-6.0, -3.0, "-0.0");
  test(-6.0, -2.0, "-0.0");
  test(-6.0, -1.0, "-0.0");
  test(-6.0, 1.0, "0.0");
  test(-6.0, 2.0, "0.0");
  test(-6.0, 3.0, "0.0");
  test(-6.0, 4.0, "2.0");
  test(-6.0, 5.0, "4.0");
  test(-6.0, 6.0, "0.0");
  test(-6.0, 7.0, "1.0");
  test(-6.0, 8.0, "2.0");
  test(-6.0, 9.0, "3.0");
  test(-6.0, 10.0, "4.0");
  test(-5.0, -10.0, "-5.0");
  test(-5.0, -9.0, "-5.0");
  test(-5.0, -8.0, "-5.0");
  test(-5.0, -7.0, "-5.0");
  test(-5.0, -6.0, "-5.0");
  test(-5.0, -5.0, "-0.0");
  test(-5.0, -4.0, "-1.0");
  test(-5.0, -3.0, "-2.0");
  test(-5.0, -2.0, "-1.0");
  test(-5.0, -1.0, "-0.0");
  test(-5.0, 1.0, "0.0");
  test(-5.0, 2.0, "1.0");
  test(-5.0, 3.0, "1.0");
  test(-5.0, 4.0, "3.0");
  test(-5.0, 5.0, "0.0");
  test(-5.0, 6.0, "1.0");
  test(-5.0, 7.0, "2.0");
  test(-5.0, 8.0, "3.0");
  test(-5.0, 9.0, "4.0");
  test(-5.0, 10.0, "5.0");
  test(-4.0, -10.0, "-4.0");
  test(-4.0, -9.0, "-4.0");
  test(-4.0, -8.0, "-4.0");
  test(-4.0, -7.0, "-4.0");
  test(-4.0, -6.0, "-4.0");
  test(-4.0, -5.0, "-4.0");
  test(-4.0, -4.0, "-0.0");
  test(-4.0, -3.0, "-1.0");
  test(-4.0, -2.0, "-0.0");
  test(-4.0, -1.0, "-0.0");
  test(-4.0, 1.0, "0.0");
  test(-4.0, 2.0, "0.0");
  test(-4.0, 3.0, "2.0");
  test(-4.0, 4.0, "0.0");
  test(-4.0, 5.0, "1.0");
  test(-4.0, 6.0, "2.0");
  test(-4.0, 7.0, "3.0");
  test(-4.0, 8.0, "4.0");
  test(-4.0, 9.0, "5.0");
  test(-4.0, 10.0, "6.0");
  test(-3.0, -10.0, "-3.0");
  test(-3.0, -9.0, "-3.0");
  test(-3.0, -8.0, "-3.0");
  test(-3.0, -7.0, "-3.0");
  test(-3.0, -6.0, "-3.0");
  test(-3.0, -5.0, "-3.0");
  test(-3.0, -4.0, "-3.0");
  test(-3.0, -3.0, "-0.0");
  test(-3.0, -2.0, "-1.0");
  test(-3.0, -1.0, "-0.0");
  test(-3.0, 1.0, "0.0");
  test(-3.0, 2.0, "1.0");
  test(-3.0, 3.0, "0.0");
  test(-3.0, 4.0, "1.0");
  test(-3.0, 5.0, "2.0");
  test(-3.0, 6.0, "3.0");
  test(-3.0, 7.0, "4.0");
  test(-3.0, 8.0, "5.0");
  test(-3.0, 9.0, "6.0");
  test(-3.0, 10.0, "7.0");
  test(-2.0, -10.0, "-2.0");
  test(-2.0, -9.0, "-2.0");
  test(-2.0, -8.0, "-2.0");
  test(-2.0, -7.0, "-2.0");
  test(-2.0, -6.0, "-2.0");
  test(-2.0, -5.0, "-2.0");
  test(-2.0, -4.0, "-2.0");
  test(-2.0, -3.0, "-2.0");
  test(-2.0, -2.0, "-0.0");
  test(-2.0, -1.0, "-0.0");
  test(-2.0, 1.0, "0.0");
  test(-2.0, 2.0, "0.0");
  test(-2.0, 3.0, "1.0");
  test(-2.0, 4.0, "2.0");
  test(-2.0, 5.0, "3.0");
  test(-2.0, 6.0, "4.0");
  test(-2.0, 7.0, "5.0");
  test(-2.0, 8.0, "6.0");
  test(-2.0, 9.0, "7.0");
  test(-2.0, 10.0, "8.0");
  test(-1.0, -10.0, "-1.0");
  test(-1.0, -9.0, "-1.0");
  test(-1.0, -8.0, "-1.0");
  test(-1.0, -7.0, "-1.0");
  test(-1.0, -6.0, "-1.0");
  test(-1.0, -5.0, "-1.0");
  test(-1.0, -4.0, "-1.0");
  test(-1.0, -3.0, "-1.0");
  test(-1.0, -2.0, "-1.0");
  test(-1.0, -1.0, "-0.0");
  test(-1.0, 1.0, "0.0");
  test(-1.0, 2.0, "1.0");
  test(-1.0, 3.0, "2.0");
  test(-1.0, 4.0, "3.0");
  test(-1.0, 5.0, "4.0");
  test(-1.0, 6.0, "5.0");
  test(-1.0, 7.0, "6.0");
  test(-1.0, 8.0, "7.0");
  test(-1.0, 9.0, "8.0");
  test(-1.0, 10.0, "9.0");
  test(0.0, -10.0, "-0.0");
  test(0.0, -9.0, "-0.0");
  test(0.0, -8.0, "-0.0");
  test(0.0, -7.0, "-0.0");
  test(0.0, -6.0, "-0.0");
  test(0.0, -5.0, "-0.0");
  test(0.0, -4.0, "-0.0");
  test(0.0, -3.0, "-0.0");
  test(0.0, -2.0, "-0.0");
  test(0.0, -1.0, "-0.0");
  test(0.0, 1.0, "0.0");
  test(0.0, 2.0, "0.0");
  test(0.0, 3.0, "0.0");
  test(0.0, 4.0, "0.0");
  test(0.0, 5.0, "0.0");
  test(0.0, 6.0, "0.0");
  test(0.0, 7.0, "0.0");
  test(0.0, 8.0, "0.0");
  test(0.0, 9.0, "0.0");
  test(0.0, 10.0, "0.0");
  test(1.0, -10.0, "-9.0");
  test(1.0, -9.0, "-8.0");
  test(1.0, -8.0, "-7.0");
  test(1.0, -7.0, "-6.0");
  test(1.0, -6.0, "-5.0");
  test(1.0, -5.0, "-4.0");
  test(1.0, -4.0, "-3.0");
  test(1.0, -3.0, "-2.0");
  test(1.0, -2.0, "-1.0");
  test(1.0, -1.0, "-0.0");
  test(1.0, 1.0, "0.0");
  test(1.0, 2.0, "1.0");
  test(1.0, 3.0, "1.0");
  test(1.0, 4.0, "1.0");
  test(1.0, 5.0, "1.0");
  test(1.0, 6.0, "1.0");
  test(1.0, 7.0, "1.0");
  test(1.0, 8.0, "1.0");
  test(1.0, 9.0, "1.0");
  test(1.0, 10.0, "1.0");
  test(2.0, -10.0, "-8.0");
  test(2.0, -9.0, "-7.0");
  test(2.0, -8.0, "-6.0");
  test(2.0, -7.0, "-5.0");
  test(2.0, -6.0, "-4.0");
  test(2.0, -5.0, "-3.0");
  test(2.0, -4.0, "-2.0");
  test(2.0, -3.0, "-1.0");
  test(2.0, -2.0, "-0.0");
  test(2.0, -1.0, "-0.0");
  test(2.0, 1.0, "0.0");
  test(2.0, 2.0, "0.0");
  test(2.0, 3.0, "2.0");
  test(2.0, 4.0, "2.0");
  test(2.0, 5.0, "2.0");
  test(2.0, 6.0, "2.0");
  test(2.0, 7.0, "2.0");
  test(2.0, 8.0, "2.0");
  test(2.0, 9.0, "2.0");
  test(2.0, 10.0, "2.0");
  test(3.0, -10.0, "-7.0");
  test(3.0, -9.0, "-6.0");
  test(3.0, -8.0, "-5.0");
  test(3.0, -7.0, "-4.0");
  test(3.0, -6.0, "-3.0");
  test(3.0, -5.0, "-2.0");
  test(3.0, -4.0, "-1.0");
  test(3.0, -3.0, "-0.0");
  test(3.0, -2.0, "-1.0");
  test(3.0, -1.0, "-0.0");
  test(3.0, 1.0, "0.0");
  test(3.0, 2.0, "1.0");
  test(3.0, 3.0, "0.0");
  test(3.0, 4.0, "3.0");
  test(3.0, 5.0, "3.0");
  test(3.0, 6.0, "3.0");
  test(3.0, 7.0, "3.0");
  test(3.0, 8.0, "3.0");
  test(3.0, 9.0, "3.0");
  test(3.0, 10.0, "3.0");
  test(4.0, -10.0, "-6.0");
  test(4.0, -9.0, "-5.0");
  test(4.0, -8.0, "-4.0");
  test(4.0, -7.0, "-3.0");
  test(4.0, -6.0, "-2.0");
  test(4.0, -5.0, "-1.0");
  test(4.0, -4.0, "-0.0");
  test(4.0, -3.0, "-2.0");
  test(4.0, -2.0, "-0.0");
  test(4.0, -1.0, "-0.0");
  test(4.0, 1.0, "0.0");
  test(4.0, 2.0, "0.0");
  test(4.0, 3.0, "1.0");
  test(4.0, 4.0, "0.0");
  test(4.0, 5.0, "4.0");
  test(4.0, 6.0, "4.0");
  test(4.0, 7.0, "4.0");
  test(4.0, 8.0, "4.0");
  test(4.0, 9.0, "4.0");
  test(4.0, 10.0, "4.0");
  test(5.0, -10.0, "-5.0");
  test(5.0, -9.0, "-4.0");
  test(5.0, -8.0, "-3.0");
  test(5.0, -7.0, "-2.0");
  test(5.0, -6.0, "-1.0");
  test(5.0, -5.0, "-0.0");
  test(5.0, -4.0, "-3.0");
  test(5.0, -3.0, "-1.0");
  test(5.0, -2.0, "-1.0");
  test(5.0, -1.0, "-0.0");
  test(5.0, 1.0, "0.0");
  test(5.0, 2.0, "1.0");
  test(5.0, 3.0, "2.0");
  test(5.0, 4.0, "1.0");
  test(5.0, 5.0, "0.0");
  test(5.0, 6.0, "5.0");
  test(5.0, 7.0, "5.0");
  test(5.0, 8.0, "5.0");
  test(5.0, 9.0, "5.0");
  test(5.0, 10.0, "5.0");
  test(6.0, -10.0, "-4.0");
  test(6.0, -9.0, "-3.0");
  test(6.0, -8.0, "-2.0");
  test(6.0, -7.0, "-1.0");
  test(6.0, -6.0, "-0.0");
  test(6.0, -5.0, "-4.0");
  test(6.0, -4.0, "-2.0");
  test(6.0, -3.0, "-0.0");
  test(6.0, -2.0, "-0.0");
  test(6.0, -1.0, "-0.0");
  test(6.0, 1.0, "0.0");
  test(6.0, 2.0, "0.0");
  test(6.0, 3.0, "0.0");
  test(6.0, 4.0, "2.0");
  test(6.0, 5.0, "1.0");
  test(6.0, 6.0, "0.0");
  test(6.0, 7.0, "6.0");
  test(6.0, 8.0, "6.0");
  test(6.0, 9.0, "6.0");
  test(6.0, 10.0, "6.0");
  test(7.0, -10.0, "-3.0");
  test(7.0, -9.0, "-2.0");
  test(7.0, -8.0, "-1.0");
  test(7.0, -7.0, "-0.0");
  test(7.0, -6.0, "-5.0");
  test(7.0, -5.0, "-3.0");
  test(7.0, -4.0, "-1.0");
  test(7.0, -3.0, "-2.0");
  test(7.0, -2.0, "-1.0");
  test(7.0, -1.0, "-0.0");
  test(7.0, 1.0, "0.0");
  test(7.0, 2.0, "1.0");
  test(7.0, 3.0, "1.0");
  test(7.0, 4.0, "3.0");
  test(7.0, 5.0, "2.0");
  test(7.0, 6.0, "1.0");
  test(7.0, 7.0, "0.0");
  test(7.0, 8.0, "7.0");
  test(7.0, 9.0, "7.0");
  test(7.0, 10.0, "7.0");
  test(8.0, -10.0, "-2.0");
  test(8.0, -9.0, "-1.0");
  test(8.0, -8.0, "-0.0");
  test(8.0, -7.0, "-6.0");
  test(8.0, -6.0, "-4.0");
  test(8.0, -5.0, "-2.0");
  test(8.0, -4.0, "-0.0");
  test(8.0, -3.0, "-1.0");
  test(8.0, -2.0, "-0.0");
  test(8.0, -1.0, "-0.0");
  test(8.0, 1.0, "0.0");
  test(8.0, 2.0, "0.0");
  test(8.0, 3.0, "2.0");
  test(8.0, 4.0, "0.0");
  test(8.0, 5.0, "3.0");
  test(8.0, 6.0, "2.0");
  test(8.0, 7.0, "1.0");
  test(8.0, 8.0, "0.0");
  test(8.0, 9.0, "8.0");
  test(8.0, 10.0, "8.0");
  test(9.0, -10.0, "-1.0");
  test(9.0, -9.0, "-0.0");
  test(9.0, -8.0, "-7.0");
  test(9.0, -7.0, "-5.0");
  test(9.0, -6.0, "-3.0");
  test(9.0, -5.0, "-1.0");
  test(9.0, -4.0, "-3.0");
  test(9.0, -3.0, "-0.0");
  test(9.0, -2.0, "-1.0");
  test(9.0, -1.0, "-0.0");
  test(9.0, 1.0, "0.0");
  test(9.0, 2.0, "1.0");
  test(9.0, 3.0, "0.0");
  test(9.0, 4.0, "1.0");
  test(9.0, 5.0, "4.0");
  test(9.0, 6.0, "3.0");
  test(9.0, 7.0, "2.0");
  test(9.0, 8.0, "1.0");
  test(9.0, 9.0, "0.0");
  test(9.0, 10.0, "9.0");
  test(10.0, -10.0, "-0.0");
  test(10.0, -9.0, "-8.0");
  test(10.0, -8.0, "-6.0");
  test(10.0, -7.0, "-4.0");
  test(10.0, -6.0, "-2.0");
  test(10.0, -5.0, "-0.0");
  test(10.0, -4.0, "-2.0");
  test(10.0, -3.0, "-2.0");
  test(10.0, -2.0, "-0.0");
  test(10.0, -1.0, "-0.0");
  test(10.0, 1.0, "0.0");
  test(10.0, 2.0, "0.0");
  test(10.0, 3.0, "1.0");
  test(10.0, 4.0, "2.0");
  test(10.0, 5.0, "0.0");
  test(10.0, 6.0, "4.0");
  test(10.0, 7.0, "3.0");
  test(10.0, 8.0, "2.0");
  test(10.0, 9.0, "1.0");
  test(10.0, 10.0, "0.0");
}

TEST(StarlarkFloat, BinaryPercentInfinity) {
  starlark_float zero(0);
  starlark_float one(1);
  starlark_float inf(std::numeric_limits<double>::infinity());
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ("0.0", zero.binary_percent(inf, ctx, error_callback)->str());
  EXPECT_EQ("1.0", one.binary_percent(inf, ctx, error_callback)->str());
  EXPECT_EQ("nan", inf.binary_percent(one, ctx, error_callback)->str());
  EXPECT_EQ("nan", inf.binary_percent(inf, ctx, error_callback)->str());
}

TEST(StarlarkFloat, BinaryPercentError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for %: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryPercentOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, BinaryPercentZeroFloatError) {
  starlark_float f0(0.0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(f0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinaryPercentZeroIntError) {
  starlark_integer i0(0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(i0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinaryPercentZeroBigintError) {
  starlark_bigint b0(number::zero());
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(b0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, PercentEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.percent_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for %=: 'float' and 'bool'");
}

}  // namespace
