////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2015, University of Washington and Battelle
// Memorial Institute.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//     * Redistributions of source code must retain the above
//       copyright notice, this list of conditions and the following
//       disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials
//       provided with the distribution.
//     * Neither the name of the University of Washington, Battelle
//       Memorial Institute, or the names of their contributors may be
//       used to endorse or promote products derived from this
//       software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// UNIVERSITY OF WASHINGTON OR BATTELLE MEMORIAL INSTITUTE BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
////////////////////////////////////////////////////////////////////////

/// This file contains tests for the GlobalAddress<> class.

#include <boost/test/unit_test.hpp>

#include "Grappa.hpp"
#include "Addressing.hpp"


BOOST_AUTO_TEST_SUITE( Addressing_tests );


struct array_element {
  Core node;
  int offset;
  int block;
  int index;
};

array_element global_array[ 1234 ] __attribute__ ((aligned (1 << 12)));


int64_t int64_array[ 32 ] __attribute__ ((aligned (1 << 12)));


struct threeword_array_element {
  int64_t a[ 3 ];
};

// test for brandonm
struct BrandonM { 
  int8_t bar[36];
};

struct BrandonM32 { 
  int8_t bar[32];
};

struct BrandonM64 { 
  int8_t bar[64];
};


BOOST_AUTO_TEST_CASE( test1 ) {
  Grappa::init( GRAPPA_TEST_ARGS );
  Grappa::run([]{
    if( 0 == Grappa::mycore() ) {
      int foo;
      int bar;
      GlobalAddress< int > foop = GlobalAddress< int >::TwoDimensional( &foo );
      GlobalAddress< int > barp = GlobalAddress< int >::TwoDimensional( &bar );
      BOOST_CHECK_EQUAL( &foo, foop.pointer() );
      BOOST_CHECK_EQUAL( &bar, barp.pointer() );
      BOOST_CHECK_EQUAL( 8, sizeof( barp ) );
    
      GlobalAddress< int > gp2 = make_global( &foo );
      BOOST_CHECK_EQUAL( gp2.pointer(), &foo );
    

      BOOST_MESSAGE("sizeof(array_element) == " << sizeof(array_element));
      BOOST_MESSAGE("Filling in array");

      const int array_size = 12;
    
      // array for comparison
      array_element array[ array_size * Grappa::cores()];
    
      // distribute elements to node arrays in round robin fashion. 
      array_element node_array[ Grappa::cores() ][ array_size ];
      // these are the indices for each node
      int node_i[ Grappa::cores() ];
      for( int i = 0; i < Grappa::cores(); ++i ) {
        node_i[i] = 0;
      }

      for( int i = 0; i < array_size * Grappa::cores(); ++i ) {
        intptr_t byte_address = i * sizeof( array_element );

        intptr_t block_index = byte_address / block_size;
        intptr_t offset_in_block = byte_address % block_size;

        Core node = block_index % Grappa::cores();
        intptr_t node_block_index = block_index / Grappa::cores();

        array[ i ].block = node_block_index; //( (i * sizeof(array_element)) / block_size ) / Grappa::cores();
        array[ i ].offset = offset_in_block; //(i * sizeof(array_element)) % block_size;
        array[ i ].node = node; //( (i * sizeof(array_element)) / block_size ) % Grappa::cores();
        //array[ i ].address = array[i].block * block_size + array[i].offset;
        array[ i ].index = i;
        //node_array[ array[i].node ][ node_i[i]++ ] = array[i];

        node_array[ node ][ node_i[ node ]++ ] = array[i];
      
        VLOG(1) << "   " << i << ":"
          //<< " local address " << array[i].address
                << " index " << array[i].index
                << " node " << array[i].node
                << " block " << array[i].block
                << " offset " << array[i].offset;
      }

      for( int node = 0; node < Grappa::cores(); ++node ) {
        VLOG(1) << "Contents of node " << node << " memory";
        for( int i = 0; i < array_size; ++i ) {
          VLOG(1) << "   " << i << ":"
            //<< " local address " << array[i].address
                  << " index " << node_array[node][i].index
                  << " node " << node_array[node][i].node
                  << " block " << node_array[node][i].block
                  << " offset " << node_array[node][i].offset;
          BOOST_CHECK_EQUAL( node_array[node][i].node, node );
          //BOOST_CHECK_EQUAL( array[i].block * block_size + array[i].offset, i * sizeof(array_element) );
        }
      }


      // hack the test
      void* prev_base = Grappa::impl::global_memory_chunk_base;
      Grappa::impl::global_memory_chunk_base = 0;


      GlobalAddress< array_element > l1 = GlobalAddress< array_element >::Linear( 0 );
      // BOOST_CHECK_EQUAL( l1.pool(), 0 );
      BOOST_CHECK_EQUAL( l1.pointer(), (array_element *) 0 );
    
      ptrdiff_t offset = &array[0] - (array_element *)NULL;
      int j = 0;
      for( int i = 0; i < array_size * Grappa::cores(); ++i ) {
        GlobalAddress< array_element > gap = l1 + i;
        ptrdiff_t node_base_address = &node_array[ array[i].node ][0] - (array_element *)NULL;
        BOOST_CHECK_EQUAL( array[i].node, gap.core() );

        array_element * ap = gap.pointer() + node_base_address;

        BOOST_CHECK_EQUAL( array[i].index, ap->index );
        BOOST_CHECK_EQUAL( array[i].node, ap->node );
        BOOST_CHECK_EQUAL( array[i].block, ap->block );
        BOOST_CHECK_EQUAL( array[i].offset, ap->offset );

      }


      // check block min block max
      {
        BOOST_CHECK_EQUAL( block_size, 64 ); // this is our assumption
      
        GlobalAddress< int64_t > a0 = make_linear( &int64_array[0] );
        GlobalAddress< int64_t > a3 = a0 + 3;
        GlobalAddress< int64_t > a7 = a0 + 7;
        GlobalAddress< int64_t > a8 = a0 + 8;
        GlobalAddress< int64_t > a15 = a0 + 15;
        GlobalAddress< int64_t > a16 = a0 + 16;
        GlobalAddress< int64_t > a24 = a0 + 24;
      
        BOOST_MESSAGE( "alignment is " << __alignof__ (int64_array) );
        BOOST_MESSAGE( "a0: " << a0);
        BOOST_MESSAGE( "a3: " << a3);
        BOOST_MESSAGE( "a7: " << a7);
        BOOST_MESSAGE( "a8: " << a8);
        BOOST_MESSAGE( "a15: " << a15);
        BOOST_MESSAGE( "a16: " << a16);
        BOOST_MESSAGE( "a24: " << a24);

        // make sure word block mins match
        BOOST_CHECK_EQUAL( a0.block_min(), a0 );
        BOOST_CHECK_EQUAL( a3.block_min(), a0 );
        BOOST_CHECK_EQUAL( a7.block_min(), a0 );
        BOOST_CHECK_EQUAL( a8.block_min(), a8 );
      
        // make sure word block maxes match
        BOOST_CHECK_EQUAL( a0.block_max(), a8 );
        BOOST_CHECK_EQUAL( a3.block_max(), a8 );
        BOOST_CHECK_EQUAL( a7.block_max(), a8 );
        BOOST_CHECK_EQUAL( a8.block_max(), a16 );

        // check larger blocks
        BOOST_CHECK_EQUAL( sizeof( threeword_array_element ), 3 * sizeof(int64_t) );
      
        BOOST_CHECK_EQUAL( sizeof( threeword_array_element * ), sizeof( int64_t * ) );
        threeword_array_element * threeword_array_ptr = reinterpret_cast< threeword_array_element * >( &int64_array[0] );
        GlobalAddress< threeword_array_element > t0 = make_linear( threeword_array_ptr );
        GlobalAddress< threeword_array_element > t1 = t0 + 1;
        GlobalAddress< threeword_array_element > t2 = t0 + 2;
        GlobalAddress< threeword_array_element > t3 = t0 + 3;
        GlobalAddress< threeword_array_element > t6 = t0 + 6;
        GlobalAddress< threeword_array_element > t7 = t0 + 7;

        BOOST_MESSAGE( "t0: " << t0);
        BOOST_MESSAGE( "t1: " << t1);
        BOOST_MESSAGE( "t2: " << t2);
        BOOST_MESSAGE( "t3: " << t3);
        BOOST_MESSAGE( "t6: " << t6);
        BOOST_MESSAGE( "t7: " << t7);

        // make sure triword block mins match
        BOOST_CHECK_EQUAL( t0.block_min(), a0 );
        BOOST_CHECK_EQUAL( t1.block_min(), a0 );
        BOOST_CHECK_EQUAL( t2.block_min(), a0 );
        BOOST_CHECK_EQUAL( t3.block_min(), a8 );
        BOOST_CHECK_EQUAL( t6.block_min(), a16 );
        BOOST_CHECK_EQUAL( t7.block_min(), a16 );
      
        // make sure triword block maxes match
        BOOST_CHECK_EQUAL( t0.block_max(), a8 );
        BOOST_CHECK_EQUAL( t1.block_max(), a8 );
        BOOST_CHECK_EQUAL( t2.block_max(), a16 );
        BOOST_CHECK_EQUAL( t3.block_max(), a16 );
        BOOST_CHECK_EQUAL( t6.block_max(), a24 );
        BOOST_CHECK_EQUAL( t7.block_max(), a24 );

      
      }

      GlobalAddress< array_element > l2 = make_linear( &global_array[0] );
      BOOST_MESSAGE( "Pointer is " << l2 );
      BOOST_CHECK_EQUAL( l2.core(), 0 );
      BOOST_CHECK_EQUAL( l2.pointer(), &global_array[0] );

      ++l2;
      BOOST_MESSAGE( "After increment, pointer is " << l2 );
      BOOST_CHECK_EQUAL( l2.core(), 0 );
      BOOST_CHECK_EQUAL( l2.pointer(), &global_array[1] );

      l2 += 3;
      BOOST_MESSAGE( "After += 3, pointer is " << l2 );
      BOOST_CHECK_EQUAL( l2.core(), 1 );
      BOOST_CHECK_EQUAL( l2.pointer(), &global_array[0] );

      ++l2;
      BOOST_MESSAGE( "After increment, pointer is " << l2 );
      BOOST_CHECK_EQUAL( l2.core(), 1 );
      BOOST_CHECK_EQUAL( l2.pointer(), &global_array[1] );

      l2 += (Grappa::cores()-1) * block_size / sizeof(array_element);
      BOOST_MESSAGE( "After += 4, pointer is " << l2 );
      BOOST_CHECK_EQUAL( l2.core(), 0 );
      BOOST_CHECK_EQUAL( l2.pointer(), &global_array[5] );

      // casting
      void * foo_pv = l2 - 4;
      void * bar_pv = l2;
      array_element * foo_p = reinterpret_cast< array_element * >( foo_pv );
      array_element * bar_p = reinterpret_cast< array_element * >( bar_pv );
      BOOST_CHECK_EQUAL( foo_p + 4, bar_p );

      // pointer to member
      GlobalAddress< array_element > l3 = make_linear( &global_array[0] );
      GlobalAddress< int > l3block = global_pointer_to_member( l3, &array_element::block );
      BOOST_CHECK_EQUAL( l3block.pointer(), &(global_array[0].block) );

      while( l3.core() == 0 ) {
        ++l3;
      }
      ++l3; // one more to put us on second element of second node
    
      l3block = global_pointer_to_member( l3, &array_element::block );
      // still in first block, so address should be same as second element on first node
      BOOST_CHECK_EQUAL( l3block.pointer(), &(global_array[1].block) ); 

      // check ordering
      BOOST_CHECK( l3 < l3+1 );


      // test for brandonm's 36-byte request bug
      {
        GlobalAddress< BrandonM > brandonm = GlobalAddress< BrandonM >::Raw( 0x2469c0000000 );
        GlobalAddress< BrandonM > brandonm_min = GlobalAddress< BrandonM >::Raw( 0x2469c0000000 );
        GlobalAddress< BrandonM > brandonm_max = GlobalAddress< BrandonM >::Raw( 0x2469c0000040 );
        BOOST_CHECK_EQUAL( sizeof( BrandonM ), 36 );
        BOOST_CHECK_EQUAL( block_size, 64 );
        BOOST_CHECK_EQUAL( brandonm.block_min(), brandonm_min );
        BOOST_CHECK_EQUAL( brandonm.block_max(), brandonm_max );
        BOOST_MESSAGE( "brandonm address is " << brandonm );
        BOOST_MESSAGE( "brandonm address + 1 is " << brandonm + 1 );
        BOOST_MESSAGE( "brandonm address + 2 - 1 is " << brandonm + 2 - 1 );
        BOOST_MESSAGE( "brandonm block_min is " << brandonm.block_min() );
        BOOST_MESSAGE( "brandonm block_max is " << brandonm.block_max() );
        BOOST_MESSAGE( "brandonm address + 1 block max is " << (brandonm + 1).block_max() );
        BOOST_MESSAGE( "brandonm address + 2 - 1 block max is " << (brandonm + 2 - 1).block_max() );
        BOOST_MESSAGE( "block_min" << GlobalAddress< BrandonM >::Raw( 0x2469c0000003 ).block_min() );
        BOOST_MESSAGE( "block_max" << GlobalAddress< BrandonM >::Raw( 0x2469c0000003 ).block_max() );

        // these tests were broken, assuming operator-() operated in
        // bytes. nope. I updated them to match the current meaning, but
        // I'm not sure what good they are now. We can think about that
        // after the paper.
        ptrdiff_t brandonm_block_diff = ( (brandonm + 1 - 1).block_max() - brandonm.block_min() );
        ptrdiff_t brandonm_byte_diff = brandonm_block_diff * block_size;
        BOOST_CHECK_EQUAL( brandonm_block_diff, 1 );
        BOOST_CHECK_EQUAL( brandonm_byte_diff, 64 );
      
        ptrdiff_t brandonm_block_diff2 = ( (brandonm + 2 - 1).block_max() - brandonm.block_min() );
        ptrdiff_t brandonm_byte_diff2 = brandonm_block_diff2 * block_size;
        BOOST_CHECK_EQUAL( brandonm_block_diff2, 3 );
        BOOST_CHECK_EQUAL( brandonm_byte_diff2, 192 );
      
        ptrdiff_t brandonm_block_diff3 = ( (brandonm + 3 - 1).block_max() - brandonm.block_min() );
        ptrdiff_t brandonm_byte_diff3 = brandonm_block_diff3 * block_size;
        BOOST_CHECK_EQUAL( brandonm_block_diff3, 3 );
        BOOST_CHECK_EQUAL( brandonm_byte_diff3, 192 );

        ptrdiff_t brandonm_block_diff4 = ( (brandonm + 4 - 1).block_max() - brandonm.block_min() );
        ptrdiff_t brandonm_byte_diff4 = brandonm_block_diff4 * block_size;
        BOOST_CHECK_EQUAL( brandonm_block_diff4, 5 );
        BOOST_CHECK_EQUAL( brandonm_byte_diff4, 320 );

        GlobalAddress< BrandonM > brandonm2 = GlobalAddress< BrandonM >::Raw( 0x2469c000007c );
        auto brandonm2_max = brandonm2.last_byte().block_max();    
        auto brandonm2_min = brandonm2.first_byte().block_min();
        BOOST_MESSAGE( "brandonm2 = " << brandonm2 );
        BOOST_MESSAGE( "brandonm2_max = " << brandonm2_max );
        BOOST_MESSAGE( "brandonm2_min = " << brandonm2_min );

        ptrdiff_t brandonm2_byte_diff = ( brandonm2.last_byte().block_max() - 
  					brandonm2.first_byte().block_min() );
        ptrdiff_t brandonm2_block_diff = brandonm2_byte_diff / block_size;

        BOOST_CHECK_EQUAL( brandonm2_byte_diff, 128 );
        BOOST_CHECK_EQUAL( brandonm2_block_diff, 2 );
      }
    }
  });
  Grappa::finalize();
}

BOOST_AUTO_TEST_SUITE_END();

