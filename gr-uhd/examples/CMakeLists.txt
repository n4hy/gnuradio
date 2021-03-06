# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup the include and linker paths
########################################################################
include_directories(
    ${GNURADIO_CORE_INCLUDE_DIRS}
    ${GR_UHD_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(${UHD_INCLUDE_DIRS})
link_directories(${UHD_LIBRARY_DIRS})

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

########################################################################
# Build executable
########################################################################
add_executable(tags_demo tags_demo.cc)
target_link_libraries(tags_demo gnuradio-uhd)

########################################################################
# Python examples
########################################################################
if(ENABLE_PYTHON)
GR_PYTHON_INSTALL(
    PROGRAMS
    fm_tx4.py
    fm_tx_2_daughterboards.py
    max_power.py
    usrp_am_mw_rcv.py
    usrp_nbfm_ptt.py
    usrp_nbfm_rcv.py
    usrp_spectrum_sense.py
    usrp_tv_rcv_nogui.py
    usrp_tv_rcv.py
    usrp_wfm_rcv2_nogui.py
    usrp_wfm_rcv_fmdet.py
    usrp_wfm_rcv_nogui.py
    usrp_wfm_rcv_pll.py
    usrp_wfm_rcv.py
    usrp_wfm_rcv_sca.py
    usrp_wxapt_rcv.py
    DESTINATION ${GR_PKG_DATA_DIR}/examples/uhd
    COMPONENT "uhd_python"
)

endif(ENABLE_PYTHON)
