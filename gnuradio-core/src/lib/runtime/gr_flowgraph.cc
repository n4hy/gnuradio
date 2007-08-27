/* -*- c++ -*- */
/*
 * Copyright 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gr_flowgraph.h>
#include <gr_io_signature.h>
#include <stdexcept>
#include <iostream>

#define GR_FLOWGRAPH_DEBUG 0

gr_edge::~gr_edge()
{
}

gr_flowgraph_sptr gr_make_flowgraph()
{
  return gr_flowgraph_sptr(new gr_flowgraph());
}

gr_flowgraph::gr_flowgraph()
{
}
  
gr_flowgraph::~gr_flowgraph()
{
}

void
gr_flowgraph::connect(const gr_endpoint &src, const gr_endpoint &dst)
{
  check_valid_port(src.block()->output_signature(), src.port());
  check_valid_port(dst.block()->input_signature(), dst.port());
  check_dst_not_used(dst);
  check_type_match(src, dst);

  // All ist klar, Herr Kommisar
  d_edges.push_back(gr_edge(src,dst));
}

void
gr_flowgraph::disconnect(const gr_endpoint &src, const gr_endpoint &dst)
{
  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++) {
    if (src == p->src() && dst == p->dst()) {
      d_edges.erase(p);
      return;
    }
  }

  throw std::invalid_argument("edge to disconnect not found");
}

void
gr_flowgraph::validate()
{
  d_blocks = calc_used_blocks();

  for (gr_basic_block_viter_t p = d_blocks.begin(); p != d_blocks.end(); p++) {
    std::vector<int> used_ports;
    int ninputs, noutputs;

    if (GR_FLOWGRAPH_DEBUG)
      std::cout << "Validating block: " << (*p) << std::endl;

    used_ports = calc_used_ports(*p, true); // inputs
    ninputs = used_ports.size();
    check_contiguity(*p, used_ports, true); // inputs

    used_ports = calc_used_ports(*p, false); // outputs
    noutputs = used_ports.size();
    check_contiguity(*p, used_ports, false); // outputs

    if (!((*p)->check_topology(ninputs, noutputs)))
      throw std::runtime_error("check topology failed");
  }
}

void
gr_flowgraph::clear()
{
  // Boost shared pointers will deallocate as needed
  d_blocks.clear();
  d_edges.clear();
}

void
gr_flowgraph::check_valid_port(gr_io_signature_sptr sig, int port)
{
  if (port < 0)
    throw std::invalid_argument("negative port number");
  if (sig->max_streams() >= 0 && port >= sig->max_streams())
    throw std::invalid_argument("port number exceeds max");
}

void
gr_flowgraph::check_dst_not_used(const gr_endpoint &dst)
{
  // A destination is in use if it is already on the edge list
  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++)
    if (p->dst() == dst)
      throw std::invalid_argument("dst already in use");
}

void
gr_flowgraph::check_type_match(const gr_endpoint &src, const gr_endpoint &dst)
{
  int src_size = src.block()->output_signature()->sizeof_stream_item(src.port());
  int dst_size = dst.block()->input_signature()->sizeof_stream_item(dst.port());

  if (src_size != dst_size)
    throw std::invalid_argument("itemsize mismatch between src and dst");
}

gr_basic_block_vector_t
gr_flowgraph::calc_used_blocks()
{
  gr_basic_block_vector_t tmp, result;
  std::insert_iterator<gr_basic_block_vector_t> inserter(result, result.begin());

  // Collect all blocks in the edge list
  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++) {
    tmp.push_back(p->src().block());
    tmp.push_back(p->dst().block());
  }

  // Return vector of unique blocks
  sort(tmp.begin(), tmp.end());
  unique_copy(tmp.begin(), tmp.end(), inserter);
  return result;
}

std::vector<int>
gr_flowgraph::calc_used_ports(gr_basic_block_sptr block, bool check_inputs)
{
  std::vector<int> tmp, result;
  std::insert_iterator<std::vector<int> > inserter(result, result.begin());

  // Collect all seen ports 
  gr_edge_vector_t edges = calc_connections(block, check_inputs);
  for (gr_edge_viter_t p = edges.begin(); p != edges.end(); p++) {
    if (check_inputs == true)
      tmp.push_back(p->dst().port());
    else
      tmp.push_back(p->src().port());
  }

  // Return vector of unique values
  std::sort(tmp.begin(), tmp.end());
  std::unique_copy(tmp.begin(), tmp.end(), inserter);
  return result;
}

gr_edge_vector_t
gr_flowgraph::calc_connections(gr_basic_block_sptr block, bool check_inputs)
{
  gr_edge_vector_t result;

  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++) {
    if (check_inputs) {
      if (p->dst().block() == block)
	result.push_back(*p);
    }
    else {
      if (p->src().block() == block)
	result.push_back(*p);
    }
  }

  return result; // assumes no duplicates
}

void
gr_flowgraph::check_contiguity(gr_basic_block_sptr block,
			       const std::vector<int> &used_ports,
			       bool check_inputs)
{
  gr_io_signature_sptr sig =
    check_inputs ? block->input_signature() : block->output_signature();

  int nports = used_ports.size();
  int min_ports = sig->min_streams();

  if (nports == 0) {
    if (min_ports == 0)
      return;
    else
      throw std::runtime_error("insufficient ports");
  }

  if (used_ports[nports-1]+1 != nports) {
    for (int i = 0; i < nports; i++)
      if (used_ports[i] != i)
	throw std::runtime_error("missing input assignment");
  }
}

gr_basic_block_vector_t
gr_flowgraph::calc_downstream_blocks(gr_basic_block_sptr block, int port)
{
  gr_basic_block_vector_t tmp, result;
  std::insert_iterator<gr_basic_block_vector_t> inserter(result, result.begin());

  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++)
    if (p->src() == gr_endpoint(block, port))
      tmp.push_back(p->dst().block());

  // Remove duplicates
  sort(tmp.begin(), tmp.end());
  unique_copy(tmp.begin(), tmp.end(), inserter);
  return result;
}

gr_basic_block_vector_t
gr_flowgraph::calc_downstream_blocks(gr_basic_block_sptr block)
{
  gr_basic_block_vector_t tmp, result;
  std::insert_iterator<gr_basic_block_vector_t> inserter(result, result.begin());

  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++)
    if (p->src().block() == block)
      tmp.push_back(p->dst().block());

  // Remove duplicates
  sort(tmp.begin(), tmp.end());
  unique_copy(tmp.begin(), tmp.end(), inserter);
  return result;
}

gr_edge_vector_t
gr_flowgraph::calc_upstream_edges(gr_basic_block_sptr block)
{
  gr_edge_vector_t result;

  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++)
    if (p->dst().block() == block)
      result.push_back(*p);

  return result; // Assume no duplicates
}

bool
gr_flowgraph::has_block_p(gr_basic_block_sptr block)
{
  gr_basic_block_viter_t result;
  result = std::find(d_blocks.begin(), d_blocks.end(), block);
  return (result != d_blocks.end());
}

gr_edge
gr_flowgraph::calc_upstream_edge(gr_basic_block_sptr block, int port)
{
  gr_edge result;

  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++) {
    if (p->dst() == gr_endpoint(block, port)) {
      result = (*p);
      break;
    }
  }

  return result;
}

std::vector<gr_basic_block_vector_t>
gr_flowgraph::partition()
{
  std::vector<gr_basic_block_vector_t> result;
  gr_basic_block_vector_t blocks = calc_used_blocks();
  gr_basic_block_vector_t graph;

  while (blocks.size() > 0) {
    graph = calc_reachable_blocks(blocks[0], blocks);
    assert(graph.size());
    result.push_back(topological_sort(graph));

    for (gr_basic_block_viter_t p = graph.begin(); p != graph.end(); p++)
      blocks.erase(find(blocks.begin(), blocks.end(), *p));
  }

  return result;
}

gr_basic_block_vector_t
gr_flowgraph::calc_reachable_blocks(gr_basic_block_sptr block, gr_basic_block_vector_t &blocks)
{
  gr_basic_block_vector_t result;

  // Mark all blocks as unvisited
  for (gr_basic_block_viter_t p = blocks.begin(); p != blocks.end(); p++)
    (*p)->set_color(gr_basic_block::WHITE);

  // Recursively mark all reachable blocks
  reachable_dfs_visit(block, blocks);

  // Collect all the blocks that have been visited
  for (gr_basic_block_viter_t p = blocks.begin(); p != blocks.end(); p++)
    if ((*p)->color() == gr_basic_block::BLACK)
      result.push_back(*p);

  return result;
}

// Recursively mark all reachable blocks from given block and block list
void 
gr_flowgraph::reachable_dfs_visit(gr_basic_block_sptr block, gr_basic_block_vector_t &blocks)
{
  // Mark the current one as visited
  block->set_color(gr_basic_block::BLACK);

  // Recurse into adjacent vertices
  gr_basic_block_vector_t adjacent = calc_adjacent_blocks(block, blocks);

  for (gr_basic_block_viter_t p = adjacent.begin(); p != adjacent.end(); p++)
    if ((*p)->color() == gr_basic_block::WHITE)
      reachable_dfs_visit(*p, blocks);
}

// Return a list of block adjacent to a given block along any edge
gr_basic_block_vector_t 
gr_flowgraph::calc_adjacent_blocks(gr_basic_block_sptr block, gr_basic_block_vector_t &blocks)
{
  gr_basic_block_vector_t tmp, result;
  std::insert_iterator<gr_basic_block_vector_t> inserter(result, result.begin());
    
  // Find any blocks that are inputs or outputs
  for (gr_edge_viter_t p = d_edges.begin(); p != d_edges.end(); p++) {

    if (p->src().block() == block)
      tmp.push_back(p->dst().block());
    if (p->dst().block() == block)
      tmp.push_back(p->src().block());
  }    

  // Remove duplicates
  sort(tmp.begin(), tmp.end());
  unique_copy(tmp.begin(), tmp.end(), inserter);
  return result;
}

gr_basic_block_vector_t
gr_flowgraph::topological_sort(gr_basic_block_vector_t &blocks)
{
  gr_basic_block_vector_t tmp;
  gr_basic_block_vector_t result;
  tmp = sort_sources_first(blocks);

  // Start 'em all white
  for (gr_basic_block_viter_t p = tmp.begin(); p != tmp.end(); p++)
    (*p)->set_color(gr_basic_block::WHITE);

  for (gr_basic_block_viter_t p = tmp.begin(); p != tmp.end(); p++) {
    if ((*p)->color() == gr_basic_block::WHITE)
      topological_dfs_visit(*p, result);
  }    

  reverse(result.begin(), result.end());
  return result;
}

gr_basic_block_vector_t
gr_flowgraph::sort_sources_first(gr_basic_block_vector_t &blocks)
{
  gr_basic_block_vector_t sources, nonsources, result;

  for (gr_basic_block_viter_t p = blocks.begin(); p != blocks.end(); p++) {
    if (source_p(*p))
      sources.push_back(*p);
    else
      nonsources.push_back(*p);
  }

  for (gr_basic_block_viter_t p = sources.begin(); p != sources.end(); p++)
    result.push_back(*p);

  for (gr_basic_block_viter_t p = nonsources.begin(); p != nonsources.end(); p++)
    result.push_back(*p);

  return result;
}

bool
gr_flowgraph::source_p(gr_basic_block_sptr block)
{
  return (calc_upstream_edges(block).size() == 0);
}

void
gr_flowgraph::topological_dfs_visit(gr_basic_block_sptr block, gr_basic_block_vector_t &output)
{
  block->set_color(gr_basic_block::GREY);
  gr_basic_block_vector_t blocks(calc_downstream_blocks(block));

  for (gr_basic_block_viter_t p = blocks.begin(); p != blocks.end(); p++) {
    switch ((*p)->color()) {
    case gr_basic_block::WHITE:           
      topological_dfs_visit(*p, output);
      break;

    case gr_basic_block::GREY:            
      throw std::runtime_error("flow graph has loops!");

    case gr_basic_block::BLACK:
      continue;

    default:
      throw std::runtime_error("invalid color on block!");
    }
  }

  block->set_color(gr_basic_block::BLACK);
  output.push_back(block);
}
