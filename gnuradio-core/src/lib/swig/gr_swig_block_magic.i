/* -*- c++ -*- */
/*
 * Copyright 2004,2010 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

%define GR_SWIG_BLOCK_MAGIC(PKG, BASE_NAME)
_GR_SWIG_BLOCK_MAGIC_HELPER(PKG, BASE_NAME, PKG ## _ ## BASE_NAME)
%enddef

%define _GR_SWIG_BLOCK_MAGIC_HELPER_COMMON(PKG, BASE_NAME, FULL_NAME)
class FULL_NAME;
typedef boost::shared_ptr<FULL_NAME> FULL_NAME ## _sptr;
%template(FULL_NAME ## _sptr) boost::shared_ptr<FULL_NAME>;
%rename(BASE_NAME) PKG ## _make_ ## BASE_NAME;
%ignore FULL_NAME;
%enddef

#ifdef SWIGPYTHON
%define _GR_SWIG_BLOCK_MAGIC_HELPER(PKG, BASE_NAME, FULL_NAME)
_GR_SWIG_BLOCK_MAGIC_HELPER_COMMON(PKG, BASE_NAME, FULL_NAME)
%pythoncode %{
FULL_NAME ## _sptr.__repr__ = lambda self: "<gr_block %s (%d)>" % (self.name(), self.unique_id ())
%}
%enddef
#endif

#ifdef SWIGGUILE
#ifdef IN_GNURADIO_CORE		// normal behavior
%define _GR_SWIG_BLOCK_MAGIC_HELPER(PKG, BASE_NAME, FULL_NAME)
_GR_SWIG_BLOCK_MAGIC_HELPER_COMMON(PKG, BASE_NAME, FULL_NAME)
/* FIXME May want to add something here to get a friendlier printed representation */
%enddef
#else				// Don't strip PKG from name
%define _GR_SWIG_BLOCK_MAGIC_HELPER(PKG, BASE_NAME, FULL_NAME)
class FULL_NAME;
typedef boost::shared_ptr<FULL_NAME> FULL_NAME ## _sptr;
%template(FULL_NAME ## _sptr) boost::shared_ptr<FULL_NAME>;
%ignore FULL_NAME;
%rename(FULL_NAME) PKG ## _make_ ## BASE_NAME;
/* FIXME May want to add something here to get a friendlier printed representation */
%enddef
#endif
#endif
