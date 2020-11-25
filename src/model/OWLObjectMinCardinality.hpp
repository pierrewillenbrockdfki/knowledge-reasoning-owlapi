#ifndef OWLAPI_MODEL_OWL_OBJECT_MIN_CARDINALITY_HPP
#define OWLAPI_MODEL_OWL_OBJECT_MIN_CARDINALITY_HPP

#include "OWLClassExpression.hpp"
#include "OWLObjectPropertyExpression.hpp"
#include "OWLMinCardinalityRestriction.hpp"
#include <owlapi/Vocabulary.hpp>

namespace owlapi {
namespace model {

class OWLObjectMinCardinality : public OWLMinCardinalityRestriction
{
public:
    OWLObjectMinCardinality(OWLObjectPropertyExpression::Ptr property, uint32_t cardinality, const OWLQualification& qualification = owlapi::vocabulary::OWL::Thing())
        : OWLMinCardinalityRestriction( dynamic_pointer_cast<OWLPropertyExpression>(property), cardinality, qualification)
    {}

    virtual ~OWLObjectMinCardinality() {}

    ClassExpressionType getClassExpressionType() const { return OBJECT_MIN_CARDINALITY; }

    bool isClassExpressionLiteral() const { return false; }
};

} // end namespace model
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_OBJECT_MIN_CARDINALITY_HPP
