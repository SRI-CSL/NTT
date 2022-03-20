#pragma once

#include <crab/domains/abstract_domain.hpp>
#include <crab/domains/intervals.hpp>

/** 
 * Abstract domain that extends classical intervals with a specialized
 * transfer function for the Longa-Naehrig reduction.
 **/
namespace ntt_verifier {

template <typename Number, typename VariableName>
class ntt_interval_domain final
  : public crab::domains::abstract_domain_api<ntt_interval_domain<Number, VariableName>> {
public:
  using ntt_interval_domain_t = ntt_interval_domain<Number, VariableName>;
  using abstract_domain_t =
    crab::domains::abstract_domain_api<ntt_interval_domain_t>;
  using typename abstract_domain_t::disjunctive_linear_constraint_system_t;
  using typename abstract_domain_t::interval_t;
  using typename abstract_domain_t::linear_constraint_system_t;
  using typename abstract_domain_t::linear_constraint_t;
  using typename abstract_domain_t::linear_expression_t;
  using typename abstract_domain_t::reference_constraint_t;
  using typename abstract_domain_t::variable_or_constant_t;
  using typename abstract_domain_t::variable_t;
  using typename abstract_domain_t::variable_vector_t;
  using typename abstract_domain_t::variable_or_constant_vector_t;  
  using number_t = Number;
  using varname_t = VariableName;

private:
  using interval_domain_t = ikos::interval_domain<number_t, varname_t>;

  interval_domain_t m_inv;

  ntt_interval_domain(interval_domain_t &&inv):
    m_inv(std::move(inv)) {}

public:
  ntt_interval_domain() {}
  ntt_interval_domain(const ntt_interval_domain_t &o) = default;
  ntt_interval_domain_t &operator=(const ntt_interval_domain_t &o) = default;
  ntt_interval_domain(ntt_interval_domain_t &&o) = default;
  ntt_interval_domain_t &operator=(ntt_interval_domain_t &&o) = default;
  
  ntt_interval_domain_t make_top() const override;
  ntt_interval_domain_t make_bottom() const override;

  void set_to_top() override;
  void set_to_bottom() override;
  
  bool is_bottom() const override;
  bool is_top() const override;

  bool operator<=(const ntt_interval_domain_t &o) const override;
  void operator|=(const ntt_interval_domain_t &o) override;
  ntt_interval_domain_t operator|(const ntt_interval_domain_t &o) const override;
  ntt_interval_domain_t operator&(const ntt_interval_domain_t &o) const override;
  ntt_interval_domain_t operator||(const ntt_interval_domain_t &o) const override;
  ntt_interval_domain_t widening_thresholds(
      const ntt_interval_domain_t &o,
      const crab::thresholds<number_t> &ts) const override;
  ntt_interval_domain_t operator&&(const ntt_interval_domain_t &o) const override;

  void operator-=(const variable_t &v) override;
  interval_t operator[](const variable_t &v) override;
  interval_t at(const variable_t &v) const override;  
  void operator+=(const linear_constraint_system_t &csts) override;
  void assign(const variable_t &x, const linear_expression_t &e) override;
  void apply(crab::domains::arith_operation_t op, const variable_t &x,
             const variable_t &y, const variable_t &z) override;
  void apply(crab::domains::arith_operation_t op, const variable_t &x,
             const variable_t &y, number_t k) override;
  void apply(crab::domains::int_conv_operation_t op, const variable_t &dst,
             const variable_t &src) override;
  void apply(crab::domains::bitwise_operation_t op, const variable_t &x,
             const variable_t &y, const variable_t &z) override;
  void apply(crab::domains::bitwise_operation_t op, const variable_t &x,
             const variable_t &y, number_t k) override;
  void select(const variable_t &lhs, const linear_constraint_t &cond,
	      const linear_expression_t &e1,  const linear_expression_t &e2) override;
  void intrinsic(std::string name,
		 const variable_or_constant_vector_t &inputs,
                 const variable_vector_t &outputs) override; 
    
  void backward_assign(const variable_t &x, const linear_expression_t &e,
                       const ntt_interval_domain_t &inv) override;
  void backward_apply(crab::domains::arith_operation_t op, const variable_t &x,
                      const variable_t &y, number_t z,
                      const ntt_interval_domain_t &inv) override;
  void backward_apply(crab::domains::arith_operation_t op, const variable_t &x,
                      const variable_t &y, const variable_t &z,
                      const ntt_interval_domain_t &inv) override;
  void backward_intrinsic(std::string name,
			  const variable_or_constant_vector_t &inputs,
                          const variable_vector_t &outputs,
                          const ntt_interval_domain_t &invariant) override;
  
  /// ntt_interval_domain implements only standard abstract operations of
  /// a numerical domain so it is intended to be used as a leaf domain
  /// in the hierarchy of domains.
  BOOL_OPERATIONS_NOT_IMPLEMENTED(ntt_interval_domain_t)
  ARRAY_OPERATIONS_NOT_IMPLEMENTED(ntt_interval_domain_t)
  REGION_AND_REFERENCE_OPERATIONS_NOT_IMPLEMENTED(ntt_interval_domain_t)

  void forget(const variable_vector_t &variables) override;
  void project(const variable_vector_t &variables) override;
  void rename(const variable_vector_t &from,
              const variable_vector_t &to) override;
  void expand(const variable_t &x, const variable_t &new_x) override;
  void normalize() override {}
  void minimize() override {}

  void write(crab::crab_os &o) const override;
  linear_constraint_system_t to_linear_constraint_system() const override;
  disjunctive_linear_constraint_system_t
  to_disjunctive_linear_constraint_system() const override;
  std::string domain_name() const override;
}; // class ntt_interval_domain
} // namespace ntt_verifier

namespace crab {
namespace domains {
template <typename Number, typename VariableName>
struct abstract_domain_traits<ntt_verifier::ntt_interval_domain<Number, VariableName>> {
  using number_t = Number;
  using varname_t = VariableName;
};
} // namespace domains
} // namespace crab
