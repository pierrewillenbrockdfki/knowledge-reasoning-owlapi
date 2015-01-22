#ifndef OWLAPI_MODEL_OWL_OBJECT_MAX_CARDINALITY_HPP
#define OWLAPI_MODEL_OWL_OBJECT_MAX_CARDINALITY_HPP

#include <owlapi/model/OWLClassExpression.hpp>
#include <owlapi/model/OWLObjectPropertyExpression.hpp>
#include <owlapi/model/OWLMaxCardinalityRestriction.hpp>
#include <owlapi/Vocabulary.hpp>

namespace owlapi {
namespace model {

class OWLObjectMaxCardinality : public OWLMaxCardinalityRestriction
{
public:
    OWLObjectMaxCardinality(OWLObjectPropertyExpression::Ptr property, uint32_t cardinality, const OWLQualification& qualification = owlapi::vocabulary::OWL::Thing())
        : OWLMaxCardinalityRestriction( boost::dynamic_pointer_cast<OWLPropertyExpression>(property), cardinality, qualification)
    {}

    ClassExpressionType getClassExpressionType() const { return OBJECT_MAX_CARDINALITY; }
    bool isAnonymous() const { return true; }
    bool isClassExpressionLiteral() const { return false; }
};

} // end namespace model
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_OBJECT_MAX_CARDINALITY_HPP
