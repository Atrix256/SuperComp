//=================================================================================
//
//  TINT.h
//
//  defines our underlying int type used for superpositional bit storage: TINT
//
//=================================================================================

#pragma once

// defines the 

#include <boost/multiprecision/cpp_int.hpp>
//#include <boost/multiprecision/tommath.hpp>
//#include <boost/multiprecision/gmp.hpp>

//typedef int64_t TINT;
//typedef boost::multiprecision::int128_t TINT;
//typedef boost::multiprecision::gmp_int TINT;
//typedef boost::multiprecision::tom_int TINT;
typedef boost::multiprecision::cpp_int TINT;

