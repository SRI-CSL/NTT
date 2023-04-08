#pragma once

#include "ntt_intervals.hpp"
#include <clam/crab/crab_lang.hh>
#include <crab/support/stats.hpp>

namespace ntt_verifier {

template <typename N, typename V>
ntt_interval_domain<N,V> ntt_interval_domain<N,V>::make_top() const {
  return ntt_interval_domain(m_inv.make_top());
}

template <typename N, typename V>
ntt_interval_domain<N,V> ntt_interval_domain<N,V>::make_bottom() const {
  return ntt_interval_domain<N,V>(m_inv.make_bottom());
}

template <typename N, typename V>
void ntt_interval_domain<N,V>::set_to_top() {
  m_inv.set_to_top();
}

template <typename N, typename V>
void ntt_interval_domain<N,V>::set_to_bottom() {
  m_inv.set_to_bottom();
}

template <typename N, typename V>
bool ntt_interval_domain<N,V>::is_bottom() const {
  return m_inv.is_bottom();
}

template <typename N, typename V>
bool ntt_interval_domain<N,V>::is_top() const {
  return m_inv.is_top();
}

template <typename N, typename V>  
bool ntt_interval_domain<N,V>::operator<=(const ntt_interval_domain<N,V> &o) const  {
  crab::CrabStats::count(domain_name() + ".count.leq");
  crab::ScopedCrabStats __st__(domain_name() + ".leq");
  return (m_inv <= o.m_inv);
}

template <typename N, typename V>  
void ntt_interval_domain<N,V>::operator|=(const ntt_interval_domain<N,V> &o)  {
  crab::CrabStats::count(domain_name() + ".count.join");
  crab::ScopedCrabStats __st__(domain_name() + ".join");
  m_inv = m_inv | o.m_inv;
}

template <typename N, typename V>  
void ntt_interval_domain<N,V>::operator&=(const ntt_interval_domain<N,V> &o)  {
  crab::CrabStats::count(domain_name() + ".count.meet");
  crab::ScopedCrabStats __st__(domain_name() + ".meet");
  m_inv = m_inv & o.m_inv;
}

template <typename N, typename V>  
ntt_interval_domain<N,V> ntt_interval_domain<N,V>::operator|(const ntt_interval_domain<N,V> &o) const  {
  crab::CrabStats::count(domain_name() + ".count.join");
  crab::ScopedCrabStats __st__(domain_name() + ".join");
  return (m_inv | o.m_inv);
}
  
template <typename N, typename V>  
ntt_interval_domain<N,V> ntt_interval_domain<N,V>::operator&(const ntt_interval_domain<N,V> &o) const  {
  crab::CrabStats::count(domain_name() + ".count.meet");
  crab::ScopedCrabStats __st__(domain_name() + ".meet");
  return (m_inv & o.m_inv);
}
  
template <typename N, typename V>  
ntt_interval_domain<N,V> ntt_interval_domain<N,V>::operator||(const ntt_interval_domain<N,V> &o) const  {
  crab::CrabStats::count(domain_name() + ".count.widening");
  crab::ScopedCrabStats __st__(domain_name() + ".widening");
  return (m_inv || o.m_inv);    
}
  
template <typename N, typename V>
ntt_interval_domain<N,V> ntt_interval_domain<N,V>::widening_thresholds(
      const ntt_interval_domain<N,V> &o,
      const crab::thresholds<number_t> &ts) const  {
  crab::CrabStats::count(domain_name() + ".count.widening");
  crab::ScopedCrabStats __st__(domain_name() + ".widening");
  return m_inv.widening_thresholds(o.m_inv, ts);
}

template <typename N, typename V>  
ntt_interval_domain<N,V> ntt_interval_domain<N,V>::operator&&(const ntt_interval_domain<N,V> &o) const  {
  crab::CrabStats::count(domain_name() + ".count.narrowing");
  crab::ScopedCrabStats __st__(domain_name() + ".narrowing");
  return (m_inv && o.m_inv);
}
  
template <typename N, typename V>  
void ntt_interval_domain<N,V>::operator-=(const variable_t &v)  {
  crab::CrabStats::count(domain_name() + ".count.forget");
  crab::ScopedCrabStats __st__(domain_name() + ".forget");
  m_inv -= v;
}
  
template <typename N, typename V>  
typename ntt_interval_domain<N,V>::interval_t
ntt_interval_domain<N,V>::operator[](const variable_t &v)  {
  return m_inv[v];
}

template <typename N, typename V>  
typename ntt_interval_domain<N,V>::interval_t
ntt_interval_domain<N,V>::at(const variable_t &v) const {
  return m_inv.at(v);
}

template <typename N, typename V>  
void ntt_interval_domain<N,V>::operator+=(const linear_constraint_system_t &csts)  {
  crab::CrabStats::count(domain_name() + ".count.add_constraints");
  crab::ScopedCrabStats __st__(domain_name() + ".add_constraints");
  m_inv += csts;
}
  
template <typename N, typename V>  
void ntt_interval_domain<N,V>::assign(const variable_t &x, const linear_expression_t &e)  {
  crab::CrabStats::count(domain_name() + ".count.assign");
  crab::ScopedCrabStats __st__(domain_name() + ".assign");  
  m_inv.assign(x, e);
}

template <typename N, typename V>  
void ntt_interval_domain<N,V>::apply(crab::domains::arith_operation_t op, const variable_t &x,
				const variable_t &y, const variable_t &z)  {
  crab::CrabStats::count(domain_name() + ".count.apply");
  crab::ScopedCrabStats __st__(domain_name() + ".apply");  
  m_inv.apply(op, x, y, z);
}

template <typename N, typename V>  
void ntt_interval_domain<N,V>::apply(crab::domains::arith_operation_t op, const variable_t &x,
				const variable_t &y, number_t k)  {
  crab::CrabStats::count(domain_name() + ".count.apply");
  crab::ScopedCrabStats __st__(domain_name() + ".apply");  
  m_inv.apply(op, x, y, k);
}

// This implements the Longa-Naehrig reduction.   
template <typename N, typename V>  
void ntt_interval_domain<N,V>::intrinsic(std::string name,
				    const variable_or_constant_vector_t &inputs,
				    const variable_vector_t &outputs)  {
  if (is_bottom()) {
    return;
  }
  
  if (name == "red" && inputs.size() == 1 && outputs.size() == 1) {
    auto red = [](number_t x) {
		 return (number_t(3) * (x & number_t(4095))) - (x >> number_t(12));
	       };
    
    auto error_if_not_variable = [&name](const variable_or_constant_t &vc) {
	 if (!vc.is_variable()) {
	   CRAB_ERROR("Intrinsics ", name, " expected a variable input");
	 }
    };      
    
    error_if_not_variable(inputs[0]);
    
    variable_t in = inputs[0].get_variable();
    variable_t out = outputs[0];
    interval_t ini = m_inv[in];
    boost::optional<number_t> a = ini.lb().number();
    boost::optional<number_t> b = ini.ub().number();
    if (a && b) {
      number_t red_lb, red_ub, d;
      // compute lower-bound
      d = *b & ~4095;
      d = (d >= *a) ? d : *a;
      red_lb = red(d);
      // compute upper-bound
      d = *a | 4095;
      d = (d <= *b) ? d : *b;
      red_ub = red(d);
      interval_t redi(red_lb, red_ub);
      m_inv.set(out, redi);
      //crab::outs() << "red(" << ini <<  ")=" << redi << "\n";
    }  else {
      //crab::outs() << "red(" << ini <<  ")=[-oo,oo]\n";
      m_inv.operator-=(out);
    }
  } else {
    CRAB_WARN("Intrinsics ", name, " not implemented by ", domain_name());
  }  
}


template <typename N, typename V>    
bool ntt_interval_domain<N,V>::entails(const linear_constraint_t &cst) const {
  return m_inv.entails(cst);
}

template <typename N, typename V>      
void ntt_interval_domain<N,V>::weak_assign(const variable_t &x, const linear_expression_t &e) {
  m_inv.weak_assign(x, e);
}

template <typename N, typename V>  
void ntt_interval_domain<N,V>::backward_intrinsic(std::string name,
					     const variable_or_constant_vector_t &inputs,
					     const variable_vector_t &outputs,
					     const ntt_interval_domain<N,V> &invariant)  {
  CRAB_WARN(domain_name(), "::", "backward_intrinsic not implemented");
}
  
template <typename N, typename V>  
void ntt_interval_domain<N,V>::backward_assign(const variable_t &x, const linear_expression_t &e,
					  const ntt_interval_domain<N,V> &inv)  {
  crab::CrabStats::count(domain_name() + ".count.backward_assign");
  crab::ScopedCrabStats __st__(domain_name() + ".backward_assign");
  CRAB_WARN(domain_name(), "::", "backward_assign not implemented");  
}

template <typename N, typename V>    
void ntt_interval_domain<N,V>::backward_apply(crab::domains::arith_operation_t op, const variable_t &x,
					 const variable_t &y, number_t z,
					 const ntt_interval_domain<N,V> &inv)  {
  crab::CrabStats::count(domain_name() + ".count.backward_apply");
  crab::ScopedCrabStats __st__(domain_name() + ".backward_apply");
  CRAB_WARN(domain_name(), "::", "backward_apply not implemented");
}

template <typename N, typename V>      
void ntt_interval_domain<N,V>::backward_apply(crab::domains::arith_operation_t op, const variable_t &x,
					 const variable_t &y, const variable_t &z,
					 const ntt_interval_domain<N,V> &inv)  {
  crab::CrabStats::count(domain_name() + ".count.backward_apply");
  crab::ScopedCrabStats __st__(domain_name() + ".backward_apply");
  CRAB_WARN(domain_name(), "::", "backward_apply not implemented");    
}

template <typename N, typename V>        
void ntt_interval_domain<N,V>::apply(crab::domains::int_conv_operation_t op, const variable_t &dst,
				const variable_t &src)  {
  m_inv.apply(op, dst, src);
}

template <typename N, typename V>          
void ntt_interval_domain<N,V>::apply(crab::domains::bitwise_operation_t op, const variable_t &x,
				const variable_t &y, const variable_t &z)  {
  crab::CrabStats::count(domain_name() + ".count.apply");
  crab::ScopedCrabStats __st__(domain_name() + ".apply");
  m_inv.apply(op, x, y, z);
}

template <typename N, typename V>            
void ntt_interval_domain<N,V>::apply(crab::domains::bitwise_operation_t op, const variable_t &x,
				const variable_t &y, number_t k)  {
  crab::CrabStats::count(domain_name() + ".count.apply");
  crab::ScopedCrabStats __st__(domain_name() + ".apply");
  m_inv.apply(op, x, y, k);
}

template <typename N, typename V>            
void ntt_interval_domain<N,V>::select(const variable_t &lhs, const linear_constraint_t &cond,
				 const linear_expression_t &e1,  const linear_expression_t &e2)  {
  crab::CrabStats::count(domain_name() + ".count.select");
  crab::ScopedCrabStats __st__(domain_name() + ".select");
  m_inv.select(lhs, cond, e1, e2);
}
  
template <typename N, typename V>            
void ntt_interval_domain<N,V>::forget(const variable_vector_t &variables)  {
  m_inv.forget(variables);
}

template <typename N, typename V>            
void ntt_interval_domain<N,V>::project(const variable_vector_t &variables)  {
  crab::CrabStats::count(domain_name() + ".count.project");
  crab::ScopedCrabStats __st__(domain_name() + ".project");
  m_inv.project(variables);
}

template <typename N, typename V>            
void ntt_interval_domain<N,V>::rename(const variable_vector_t &from,
				 const variable_vector_t &to)  {
  crab::CrabStats::count(domain_name() + ".count.rename");
  crab::ScopedCrabStats __st__(domain_name() + ".rename");
  m_inv.rename(from, to);
}
  
template <typename N, typename V>            
void ntt_interval_domain<N,V>::expand(const variable_t &x, const variable_t &new_x)  {
  crab::CrabStats::count(domain_name() + ".count.expand");
  crab::ScopedCrabStats __st__(domain_name() + ".expand");
  m_inv.expand(x, new_x);
}
  
template <typename N, typename V>            
void ntt_interval_domain<N,V>::write(crab::crab_os &o) const  {
  crab::CrabStats::count(domain_name() + ".count.write");
  crab::ScopedCrabStats __st__(domain_name() + ".write");
  m_inv.write(o);
}

template <typename N, typename V>            
typename ntt_interval_domain<N,V>::linear_constraint_system_t
ntt_interval_domain<N,V>::to_linear_constraint_system() const  {
  crab::CrabStats::count(domain_name() +
			 ".count.to_linear_constraint_system");
  crab::ScopedCrabStats __st__(domain_name() +
			       ".to_linear_constraint_system");
  return m_inv.to_linear_constraint_system();
}

template <typename N, typename V>            
typename ntt_interval_domain<N,V>::disjunctive_linear_constraint_system_t
ntt_interval_domain<N,V>::to_disjunctive_linear_constraint_system() const  {
  return m_inv.to_disjunctive_linear_constraint_system();
}
  
template <typename N, typename V>            
std::string ntt_interval_domain<N,V>::domain_name() const  {
  return "NTT-Intervals";
}
} // namespace ntt_verifier

namespace ntt_verifier {
template class ntt_interval_domain<clam::number_t, clam::varname_t>;
} // namespace ntt_verifier
