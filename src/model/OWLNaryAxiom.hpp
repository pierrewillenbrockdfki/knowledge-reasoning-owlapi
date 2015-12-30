#ifndef OWLAPI_MODEL_OWL_NARY_AXIOM_HPP
#define OWLAPI_MODEL_OWL_NARY_AXIOM_HPP

#include <owlapi/model/OWLAxiom.hpp>

namespace owlapi {
namespace model {

/**
 * Represents an interface definition for an axiom that contains two or more operands that could also be represented with multiple pairwise axioms
 */
class OWLNaryAxiom
{
public:
    typedef shared_ptr<OWLNaryAxiom> Ptr;

    virtual ~OWLNaryAxiom() {}

    virtual std::vector<OWLAxiom::Ptr> asPairwiseAxioms() = 0;
};

} // end namespace model
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_NARY_AXIOM_HPP
