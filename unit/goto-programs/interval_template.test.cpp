#include <goto-programs/abstract-interpretation/interval_template.h>
#include <goto-programs/abstract-interpretation/wrapped_interval.h>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "c_types.h"

TEST_CASE(
  "Interval templates base functions are working for int",
  "[ai][interval-analysis]")
{
  interval_templatet<int> A;
  SECTION("Initialization tests")
  {
    REQUIRE(A.is_top());
    REQUIRE_FALSE(A.is_bottom());
    REQUIRE_FALSE(A.singleton());
  }
  SECTION("Lower bound is set")
  {
    A.make_ge_than(-3);
    REQUIRE_FALSE(A.is_top());
    REQUIRE_FALSE(A.is_bottom());
    REQUIRE_FALSE(A.singleton());
  }
  SECTION("Upper bound is set (singleton)")
  {
    A.make_ge_than(-3);
    A.make_le_than(0);
    REQUIRE_FALSE(A.is_top());
    REQUIRE_FALSE(A.is_bottom());
    REQUIRE_FALSE(A.singleton());
  }
  SECTION("Upper bound is set (singleton)")
  {
    A.make_ge_than(-3);
    A.make_le_than(-3);
    REQUIRE_FALSE(A.is_top());
    REQUIRE_FALSE(A.is_bottom());
    REQUIRE(A.singleton());
  }
  SECTION("Contradiction in set")
  {
    A.make_ge_than(0);
    A.make_le_than(-1);
    REQUIRE_FALSE(A.is_top());
    REQUIRE(A.is_bottom());
    REQUIRE_FALSE(A.singleton());
  }
}

TEST_CASE(
  "Interval templates relation functions are working for int",
  "[ai][interval-analysis]")
{
  interval_templatet<int> A;
  interval_templatet<int> B;

  SECTION("Union between two tops is a top")
  {
    REQUIRE(A.is_top());
    REQUIRE(B.is_top());
    A.join(B);
    REQUIRE(A.is_top());
  }

  SECTION("Union with a top results in a top")
  {
    A.make_ge_than(0);
    REQUIRE_FALSE(A.is_top());
    REQUIRE(B.is_top());
    A.join(B);
    REQUIRE(A.is_top());
  }

  SECTION("Intersection with a top results in itself")
  {
    A.make_ge_than(0);
    REQUIRE_FALSE(A.is_top());
    interval_templatet<int> previous = A;
    REQUIRE(B.is_top());
    A.meet(B);
    REQUIRE(A == previous);
  }

  SECTION("Union with a bottom results in itself")
  {
    A.make_ge_than(0);
    REQUIRE_FALSE(A.is_top());
    interval_templatet<int> previous = A;

    B.make_ge_than(1);
    B.make_le_than(0);
    REQUIRE(B.is_bottom());
    A.join(B);
    REQUIRE(A == previous);
  }

  SECTION("Intersect with a bottom results in a bottom")
  {
    REQUIRE(A.is_top());

    B.make_ge_than(1);
    B.make_le_than(0);
    REQUIRE(B.is_bottom());
    A.meet(B);
    REQUIRE(A.is_bottom());
  }

  SECTION("Union of two sets should over-aproximate")
  {
    // [0,4] U [10,20] = [0,20]
    A.make_ge_than(0);
    A.make_le_than(4);

    B.make_ge_than(10);
    B.make_le_than(20);

    interval_templatet<int> expected;
    expected.make_ge_than(0);
    expected.make_le_than(20);

    A.approx_union_with(B);
    REQUIRE(A == expected);
  }

  SECTION("Intersect should... intersect")
  {
    // [0,15] intersec [10,20] = [10,15]
    A.make_ge_than(0);
    A.make_le_than(15);

    B.make_ge_than(10);
    B.make_le_than(20);

    interval_templatet<int> expected;
    expected.make_ge_than(10);
    expected.make_le_than(15);

    A.meet(B);
    REQUIRE(A == expected);
  }

  SECTION("Intersect should... intersect 2")
  {
    // [0,5] intersec [10,20] = empty
    A.make_ge_than(0);
    A.make_le_than(5);

    B.make_ge_than(10);
    B.make_le_than(20);

    A.meet(B);
    REQUIRE(A.is_bottom());
  }

  SECTION("Add test 1")
  {
    // A: [1,10], B: [5,10], Result: [6,20]
    A.make_ge_than(1);
    A.make_le_than(10);

    B.make_ge_than(5);
    B.make_le_than(10);

    auto result = A + B;
    REQUIRE(result.lower_set);
    REQUIRE(result.upper_set);
    REQUIRE(result.lower == 6);
    REQUIRE(result.upper == 20);
  }

  SECTION("Add test 2")
  {
    // A: [-infinity,10], B: [5,10], Result: [-infinity,20]
    A.make_le_than(10);

    B.make_ge_than(5);
    B.make_le_than(10);

    auto result = A + B;
    REQUIRE_FALSE(result.lower_set);
    REQUIRE(result.upper_set);
    REQUIRE(result.upper == 20);
  }

  SECTION("Add test 3")
  {
    // A: [-infinity,10], B: [5,+infinity], Result: [-infinity,+infinity]
    A.make_le_than(10);

    B.make_ge_than(5);

    auto result = A + B;
    REQUIRE_FALSE(result.lower_set);
    REQUIRE_FALSE(result.upper_set);
  }

  SECTION("Sub test 1")
  {
    // A: [1,10], B: [5,10], Result: [-9,5]
    A.make_ge_than(1);
    A.make_le_than(10);

    B.make_ge_than(5);
    B.make_le_than(10);

    auto result = A - B;
    REQUIRE(result.lower_set);
    REQUIRE(result.upper_set);
    REQUIRE(result.lower == -9);
    REQUIRE(result.upper == 5);
  }

  SECTION("Sub test 2")
  {
    // A: [-infinity,10], B: [5,10], Result: [-infinity,5]
    A.make_le_than(10);

    B.make_ge_than(5);
    B.make_le_than(10);

    auto result = A - B;
    REQUIRE_FALSE(result.lower_set);
    REQUIRE(result.upper_set);
    REQUIRE(result.upper == 5);
  }

  SECTION("Sub test 3")
  {
    // A: [-infinity,10], B: [5,+infinity], Result: [-infinity,+infinity]
    A.make_le_than(10);

    B.make_ge_than(5);

    auto result = A - B;
    REQUIRE_FALSE(result.lower_set);
    REQUIRE(result.upper_set);
    REQUIRE(result.upper == 5);
  }

  SECTION("Mul test 1")
  {
    // A: [5,10], B: [-1,1], Result: [-10,+10]
    A.make_le_than(10);
    A.make_ge_than(5);
    B.make_ge_than(-1);
    B.make_le_than(1);

    auto result = A * B;
    REQUIRE(result.lower_set);
    REQUIRE(result.upper_set);
    REQUIRE(result.lower == -10);
    REQUIRE(result.upper == 10);
  }

  SECTION("Mul test 2")
  {
    // A: [-15,10], B: [-1,2], Result: [-30,+20]
    A.make_le_than(10);
    A.make_ge_than(-15);
    B.make_ge_than(-1);
    B.make_le_than(2);

    auto result = A * B;
    REQUIRE(result.lower_set);
    REQUIRE(result.upper_set);
    REQUIRE(result.lower == -30);
    REQUIRE(result.upper == 20);
  }

  SECTION("Div test 1")
  {
    // A: [4,10], B: [1,2], Result: [2,+10]
    A.make_le_than(10);
    A.make_ge_than(4);
    B.make_ge_than(1);
    B.make_le_than(2);

    auto result = A / B;
    REQUIRE(result.lower_set);
    REQUIRE(result.upper_set);
    REQUIRE(result.lower == 2);
    REQUIRE(result.upper == 10);
  }


  SECTION("Copy constructor")
  {
    // Just to be sure
    auto tmp_a = A;
    A.make_ge_than(0);
    REQUIRE(!tmp_a.lower_set);
    REQUIRE(A.lower_set);
  }
  SECTION("Contractor")
  {
    // From PRDC paper
    // X1: [0,20]
    // X2: [0, infinity]
    // X1 >= X2 ==> X2 <= X1 (which is what we are going to contract)
    // The result should be: A: [0,20] and B: [0,20]

    A.make_ge_than(0);
    A.make_le_than(20);
    B.make_ge_than(0);

    interval_templatet<int>::contract_interval_le(B, A);
    REQUIRE(A.lower_set);
    REQUIRE(A.upper_set);
    REQUIRE(A.lower == 0);
    REQUIRE(A.upper == 20);

    REQUIRE(B.lower_set);
    REQUIRE(B.upper_set);
    REQUIRE(B.lower == 0);
    REQUIRE(B.upper == 20);
  }
}

TEST_CASE("Wrapped Intervals tests", "[ai][interval-analysis]")
{
  config.ansi_c.set_data_model(configt::ILP32);
  unsigned N1 = 8;
  auto t1_unsigned = get_uint_type(N1);
  auto t1_signed = get_int_type(N1);

  unsigned N2 = 16;
  auto t2_unsigned = get_uint_type(N2);
  auto t2_signed = get_int_type(N2);

  SECTION("Upper bound test")
  {
    unsigned actual =
      wrapped_interval::get_upper_bound(t1_unsigned).to_uint64();
    unsigned expected = pow(2, N1);

    REQUIRE(actual == expected);
    REQUIRE(
      wrapped_interval::get_upper_bound(t1_unsigned) ==
      wrapped_interval::get_upper_bound(t1_signed));

    REQUIRE(
      wrapped_interval::get_upper_bound(t2_unsigned) ==
      wrapped_interval::get_upper_bound(t2_signed));

    REQUIRE(
      wrapped_interval::get_upper_bound(t1_unsigned) !=
      wrapped_interval::get_upper_bound(t2_signed));
  }

  SECTION("Wrapped less")
  {
    BigInt value1(10);
    BigInt value2(130);

    REQUIRE(wrapped_interval::wrapped_le(value1, 0, value2, t1_unsigned));
    REQUIRE(wrapped_interval::wrapped_le(value1, 0, value1, t1_unsigned));
    REQUIRE(!wrapped_interval::wrapped_le(value2, 0, value1, t1_unsigned));
  }

  SECTION("Init")
  {
    wrapped_interval A(t1_unsigned);
    wrapped_interval C(t2_unsigned);

    REQUIRE(A.lower == 0);
    REQUIRE(A.upper.to_uint64() == pow(2, N1) - 1);
    REQUIRE(!A.is_bottom());
    CAPTURE(A.cardinality());
    REQUIRE(A.is_top());

    REQUIRE(C.lower == 0);
    REQUIRE(C.upper.to_uint64() == pow(2, N2) - 1);
    REQUIRE(!C.is_bottom());
    REQUIRE(C.is_top());

    // Char initialization
    for(int c = 0; c < 256; c++)
    {
      A.set_lower(c);
      A.set_upper(c);
      REQUIRE(A.lower >= 0);
      REQUIRE(A.upper >= 0);
      REQUIRE(A.get_lower() == c);
      REQUIRE(A.get_upper() == c);
    }
  }

  SECTION("Init Signed")
  {
    wrapped_interval A(t1_signed);
    CAPTURE(A.lower.to_int64(), A.upper.to_uint64());
    REQUIRE(A.lower == 0);
    REQUIRE(A.upper.to_uint64() == pow(2, N1) - 1);
    REQUIRE(!A.is_bottom());
    REQUIRE(A.is_top());

    // Char initialization
    for(int c = -128; c < 128; c++)
    {
      A.set_lower(c);
      A.set_upper(c);
      REQUIRE(A.lower >= 0);
      REQUIRE(A.upper >= 0);
      REQUIRE(A.get_lower() == c);
      REQUIRE(A.get_upper() == c);
    }
  }

  SECTION("Relational Operators (Unsigned)")
  {
    wrapped_interval A(t1_unsigned);
    // [10, 250]
    A.set_lower(10);
    A.set_upper(250);
    A.make_le_than(50);
    REQUIRE(A.get_upper() == 50);
    REQUIRE(A.get_lower() == 10);

    // A: [250, 20]
    A.set_upper(20);
    A.set_lower(250);

    A.make_le_than(50); // [0, 20]

    REQUIRE(A.get_upper() == 20);
    A.make_ge_than(10);

    REQUIRE(A.get_lower() == 10);

  }

  SECTION("Relational Operators (Signed)")
  {
    wrapped_interval A(t1_signed);
    // A: [-128,127]

    REQUIRE(A.get_upper() == -1);
    REQUIRE(A.get_lower() == 0);

    A.make_le_than(500);
    A.make_le_than(50);
    // [-128,50]
    REQUIRE(A.get_upper() == 50);
    A.make_ge_than(-120);
    REQUIRE(A.get_lower() == -120);

  }

  SECTION("Non-overlap Interval")
  {
    wrapped_interval A(t1_unsigned);
    A.lower = 10;
    A.upper = 20;
    REQUIRE(A.contains(15));
    REQUIRE(!A.contains(150));
  }

  SECTION("Wrapping Interval")
  {
    wrapped_interval A(t1_unsigned);
    A.lower = 20;
    A.upper = 10;
    REQUIRE(!A.contains(15));
    REQUIRE(A.contains(25));
  }

  wrapped_interval A(t1_unsigned);
  wrapped_interval B(t1_unsigned);
  wrapped_interval As(t1_signed);
  wrapped_interval Bs(t1_signed);


  SECTION("Join/Meet when A is in B")
  {
    A.lower = 30;
    A.upper = 90;
    B.lower = 10;
    B.upper = 150;

    auto over_meet = wrapped_interval::over_meet(A, B);
    auto over_join = wrapped_interval::over_join(A, B);
    auto intersection = wrapped_interval::intersection(A, B);

    REQUIRE(over_meet.is_equal(A));
    REQUIRE(over_join.is_equal(B));
    REQUIRE(intersection.is_equal(A));
  }

  SECTION("Join/Meet from bug")
  {
    As.set_lower(-52);
    As.set_upper(-52);

    Bs.set_lower(51);
    Bs.set_upper(51);

    auto over_meet = wrapped_interval::over_meet(As, Bs);

    REQUIRE(over_meet.contains(-52));
    REQUIRE(over_meet.contains(51));
  }

  SECTION("Join/Meet when A do not overlap and B overlaps meets A in both ends")
  {
    A.lower = 10;
    A.upper = 190;
    B.lower = 150;
    B.upper = 20;
    // [10-190], [150-20]
    auto over_meet = wrapped_interval::over_meet(A, B);
    auto under_meet = wrapped_interval::under_meet(A, B);
    auto over_join = wrapped_interval::over_join(A, B);
    auto intersection = wrapped_interval::intersection(A, B);

    REQUIRE(over_join.is_top());
    REQUIRE(!under_meet.is_top());
    // Real intersection is: [150, 190] U [10, 20]
    wrapped_interval check(t1_unsigned);
    check.lower = 150;
    check.upper = 190;
    REQUIRE(check.is_included(intersection));
    check.lower = 10;
    check.upper = 20;
    REQUIRE(check.is_included(intersection));
  }
  SECTION("Join/Meet when A do not overlap and B overlaps meets A in one end")
  {
    A.lower = 10;
    A.upper = 150;
    B.lower = 200;
    B.upper = 100;
    // [10-150], [200-100]
    auto over_meet = wrapped_interval::over_meet(A, B);
    auto under_meet = wrapped_interval::under_meet(A, B);
    auto over_join = wrapped_interval::over_join(A, B);

    REQUIRE(!over_join.is_top());
    REQUIRE(over_join.lower == 200);
    REQUIRE(over_join.upper == 150);
    REQUIRE(under_meet == over_meet);
    REQUIRE(under_meet.lower == 10);
    REQUIRE(under_meet.upper == 100);

    // Intersection should be [10, 100]
    auto intersection = wrapped_interval::intersection(A, B);
    wrapped_interval check(t1_unsigned);
    check.lower = 10;
    check.upper = 100;
    REQUIRE(check.is_included(intersection));
  }

  SECTION("Join/Meet when A do not overlap and B overlaps and no intersection")
  {
    A.lower = 20;
    A.upper = 100;
    B.lower = 200;
    B.upper = 10;
    // [20-100], [200-10]
    auto over_meet = wrapped_interval::over_meet(A, B);
    auto under_meet = wrapped_interval::under_meet(A, B);
    auto over_join = wrapped_interval::over_join(A, B);

    REQUIRE(!over_join.is_top());
    REQUIRE(over_join.lower == 200);
    REQUIRE(over_join.upper == 100);

    // Intersection should be bottom
    auto intersection = wrapped_interval::intersection(A, B);
    REQUIRE(intersection.is_bottom());
  }

  SECTION("Addition Unsigned")
  {
    REQUIRE(A.lower == 0);
    REQUIRE(A.upper.to_uint64() == pow(2, N1) - 1);
    REQUIRE(!A.is_bottom());
    REQUIRE(A.is_top());

    // No Wrap
    A.set_lower(100);
    A.set_upper(150);

    B.set_lower(0);
    B.set_upper(1);

    auto C = A + B;
    REQUIRE(C.get_lower() == 100);
    REQUIRE(C.get_upper() == 151);

    // Wrap around
    A.set_lower(100);
    A.set_upper((long long)(pow(2,N1)-1));

    B.set_lower(1);
    B.set_upper(2);

    C = A + B;
    REQUIRE(C.get_lower() == 101);
    REQUIRE(C.get_upper() == 1);
  }

  SECTION("Addition Signed")
  {
    REQUIRE(As.contains((long long) -pow(2, N1)/2));
    REQUIRE(As.contains((long long) pow(2, N1)/2 - 1));
    REQUIRE(!As.is_bottom());
    REQUIRE(As.is_top());

    // No Wrap
    As.set_lower(-100);
    As.set_upper(120);

    Bs.set_lower(-5);
    Bs.set_upper(5);


    auto C = As + Bs;
    As.dump();
    Bs.dump();
    C.dump();
    CAPTURE(As.cardinality(), Bs.cardinality());
    REQUIRE(C.get_lower() == -105);
    REQUIRE(C.get_upper() == 125);

    // Wrap around
    As.set_lower(100);
    As.set_upper((long long)(pow(2,N1-1)-1));

    Bs.set_lower(-1);
    Bs.set_upper(1);

    C = As + Bs;
    REQUIRE(C.get_lower() == 99);
    REQUIRE(C.get_upper().to_int64() == -pow(2, N1)/2);
  }

}

TEST_CASE(
  "Interval templates arithmetic operations",
  "[ai][interval-analysis]")
{
  config.ansi_c.set_data_model(configt::ILP32);
  unsigned N1 = 8;
  auto t1_unsigned = get_uint_type(N1);
  auto t1_signed = get_int_type(N1);


  SECTION("North Pole")
  {
    auto np = wrapped_interval::north_pole(t1_signed);
    REQUIRE((np.lower == 127 && np.upper == 128));
  }

  SECTION("South Pole")
  {
    auto np = wrapped_interval::south_pole(t1_signed);
    REQUIRE((np.upper == 0 && np.lower == 255));
  }

  SECTION("North Split")
  {
    wrapped_interval w1(t1_signed);
    w1.lower = 100;
    w1.upper = 120;

    auto w1_split = w1.nsplit();
    REQUIRE(w1_split.size() == 1);
    REQUIRE(w1_split[0] == w1);

    w1.lower = 100;
    w1.upper = 150;
    w1_split = w1.nsplit();
    REQUIRE(w1_split.size() == 2);

    // The right part is returned as first element! (not a rule though)
    REQUIRE(w1_split[0].lower == 128);
    REQUIRE(w1_split[0].upper == 150);
    REQUIRE(w1_split[1].lower == 100);
    REQUIRE(w1_split[1].upper == 127);
  }

  SECTION("South Split")
  {
    wrapped_interval w1(t1_signed);
    w1.lower = 100;
    w1.upper = 250;

    auto w1_split = w1.ssplit();
    REQUIRE(w1_split.size() == 1);
    REQUIRE(w1_split[0] == w1);

    w1.lower = 200;
    w1.upper = 150;
    w1_split = w1.ssplit();
    REQUIRE(w1.contains(0));
    REQUIRE(w1.contains(255));
    REQUIRE(w1_split.size() == 2);

    // The left part is returned as first element! (not a rule though)
    REQUIRE(w1_split[0].lower == 0);
    REQUIRE(w1_split[0].upper == 150);
    REQUIRE(w1_split[1].lower == 200);
    REQUIRE(w1_split[1].upper == 255);
  }

  SECTION("Cut")
  {
    wrapped_interval w(t1_signed);
    w.lower = 255;
    w.upper = 129;

    auto cut = wrapped_interval::cut(w);

    bool check1 = false;
    bool check2 = false;
    bool check3 = false;

    wrapped_interval w1(t1_signed),w2(t1_signed), w3(t1_signed);
    w1.lower = 255;
    w1.upper = 255;

    w2.lower = 0;
    w2.upper = 127;

    w3.lower = 128;
    w3.upper = 129;

    // There is no way for me to check the order
    for(auto &c: cut)
    {
      if(c == w1) check1=true;
      if(c == w2) check2=true;
      if(c == w3) check3=true;
    }

    REQUIRE(check1);
    REQUIRE(check2);
    REQUIRE(check3);
  }

  SECTION("MSB")
  {
    wrapped_interval w(t1_signed);
    w.lower = 255;
    w.upper = 127;

    REQUIRE(w.most_significant_bit(w.lower));
    REQUIRE(!w.most_significant_bit(w.upper));

    w.lower = 25;
    w.upper = 130;

    REQUIRE(!w.most_significant_bit(w.lower));
    REQUIRE(w.most_significant_bit(w.upper));
  }

  SECTION("Difference")
  {
    wrapped_interval w1(t1_signed);
    w1.lower = 100;
    w1.upper = 120;

    wrapped_interval w2(t1_signed);
    w2.lower = 90;
    w2.upper = 110;


    auto result = w1.difference(w1,w2);
    CAPTURE(result.lower, result.upper);
    REQUIRE(result.lower == 111);
    REQUIRE(result.upper == 120);
  }
}

TEST_CASE(
  "Interval templates multiplication",
  "[ai][interval-analysis]")
{
  config.ansi_c.set_data_model(configt::ILP32);
  unsigned N1 = 8;
  auto t1_unsigned = get_uint_type(N1);
  auto t1_signed = get_int_type(N1);

  SECTION("Multiply unsigned")
  {
    wrapped_interval w1(t1_unsigned);
    w1.lower = 5;
    w1.upper = 10;

    wrapped_interval w2(t1_unsigned);
    w2.lower = 2;
    w2.upper = 10;

    auto w3 = w1 * w2;

    CAPTURE(w3.lower, w3.upper);
    REQUIRE(w3.lower == 10);
    REQUIRE(w3.upper == 100);

    w1.lower = 1;
    w1.upper = 2;

    w2.lower = 1;
    w2.upper = 250;

    w3 = w1 * w2;
    CAPTURE(w3.lower, w3.upper);
    REQUIRE(w3.lower == 0);
    REQUIRE(w3.upper == 254);
  }

  SECTION("Multiply signed")
  {
    wrapped_interval w1(t1_signed);
    w1.lower = 1;
    w1.upper = 10;

    wrapped_interval w2(t1_signed);
    w2.lower = 255;
    w2.upper = 255;

    auto w3 = w1 * w2;

    CAPTURE(w3.lower, w3.upper);
    REQUIRE(w3.lower == 246);
    REQUIRE(w3.upper == 255);
  }
}

TEST_CASE(
  "Interval templates division",
  "[ai][interval-analysis]")
{
  config.ansi_c.set_data_model(configt::ILP32);
  unsigned N1 = 8;
  auto t1_unsigned = get_uint_type(N1);
  auto t1_signed = get_int_type(N1);

  SECTION("Division unsigned")
  {
    wrapped_interval w1(t1_unsigned);
    w1.lower = 5;
    w1.upper = 20;

    wrapped_interval w2(t1_unsigned);
    w2.lower = 2;
    w2.upper = 10;

    auto w3 = w1 / w2;

    CAPTURE(w3.lower, w3.upper);
    REQUIRE(w3.lower == 0);
    REQUIRE(w3.upper == 10);

    w1.lower = 10;
    w1.upper = 20;

    w2.lower = 2;
    w2.upper = 10;

    w3 = w1 / w2;
    CAPTURE(w3.lower, w3.upper);
    REQUIRE(w3.lower == 1);
    REQUIRE(w3.upper == 10);
  }

  SECTION("Division signed")
  {
    wrapped_interval w1(t1_signed);
    w1.lower = 10;
    w1.upper = 20; // 10, 20

    wrapped_interval w2(t1_signed);
    w2.lower = 254; // -2
    w2.upper = 255; // -1
    // -5 (251), -20: (236)
    auto w3 = w1 / w2;

    CAPTURE(w3.lower, w3.upper);
    REQUIRE(w3.lower == 236);
    REQUIRE(w3.upper == 251);
  }
}

TEST_CASE(
  "Wrapped Interval Typecast",
  "[ai][interval-analysis]")
{
  config.ansi_c.set_data_model(configt::ILP32);
  unsigned N1 = 8, N2 = 16;
  auto t1_unsigned = get_uint_type(N1);
  auto t1_signed = get_int_type(N1);
  auto t2_unsigned = get_uint_type(N2);
  auto t2_signed = get_int_type(N2);

  SECTION("Truncation (unsigned)")
  {
    wrapped_interval w1(t2_unsigned);
    w1.lower = 30;
    w1.upper = 250;

    auto result1 = w1.trunc(t1_unsigned);
    CAPTURE(result1.lower, result1.upper);
    REQUIRE(result1.lower == 30);
    REQUIRE(result1.upper == 250);

    w1.lower = 256;
    w1.upper = 260;

    result1 = w1.trunc(t1_unsigned);
    CAPTURE(result1.lower, result1.upper);
    REQUIRE(result1.lower == 0);
    REQUIRE(result1.upper == 4);
  }

  SECTION("Truncation (signed)")
  {
    wrapped_interval w1(t2_signed);
    w1.lower = 30;
    w1.upper = 128;

    auto result1 = w1.trunc(t1_signed);
    CAPTURE(result1.lower, result1.upper);
    REQUIRE(result1.lower == 30);
    REQUIRE(result1.upper == 128);

    w1.lower = 30;
    w1.upper = 257;

    result1 = w1.trunc(t1_signed);
    CAPTURE(result1.lower, result1.upper);
    REQUIRE(result1.lower == 30);
    REQUIRE(result1.upper == 1);
  }
}

