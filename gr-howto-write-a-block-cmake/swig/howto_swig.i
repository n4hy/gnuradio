/* -*- c++ -*- */

#define HOWTO_API

%include "gnuradio.i"			// the common stuff

%{
#include "howto_square_ff.h"
#include "howto_square2_ff.h"
%}

GR_SWIG_BLOCK_MAGIC(howto,square_ff);
%include "howto_square_ff.h"

GR_SWIG_BLOCK_MAGIC(howto,square2_ff);
%include "howto_square2_ff.h"

#if SWIGGUILE
%scheme %{
(load-extension-global "libguile-gnuradio-howto_swig" "scm_init_gnuradio_howto_swig_module")
%}

%goops %{
(use-modules (gnuradio gnuradio_core_runtime))
%}
#endif
