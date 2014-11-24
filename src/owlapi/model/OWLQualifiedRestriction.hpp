#ifndef OWL_API_MODEL_QUALIFIED_RESTRICTION_HPP
#define OWL_API_MODEL_QUALIFIED_RESTRICTION_HPP

#include <owl_om/owlapi/model/IRI.hpp>
#include <owl_om/owlapi/model/OWLRestriction.hpp>
#include <owl_om/Vocabulary.hpp>

namespace owlapi {
namespace model {

typedef IRI OWLQualification;

class OWLQualifiedRestriction : public OWLRestriction
{
    OWLQualification mQualification;
    bool mQualified;

public:
    /**
     * \brief A qualified restriction allows to associate a property with a qualification
     * \details According to w3c the qualification here should be:
     * for ObjectProperties this has to be a ClassExpression (:= Class | ObjectIntersectionOf | ObjectUnionOf | ObjectComplementOf | ObjectOneOf | ObjectSomeValuesFrom | ObjectAllValuesFrom | ObjectHasValue | ObjectHasSelf)
     * for DataProperties this has to be a DataRange (:= Datatype | DataIntersectionOf | DataUnionOf | DataComplementOf | DataOneOf | DatatypeRestriction )
     * However, to facilitate implementation, we relax/generalize the requirement to an IRI, so that OWLQualification := IRI
     *
     * To qualify ObjectProperties and DataProperties they have to differ from the TOP concepts, i.e. owl:Thing and rdfs::Literal respectively
     * 
     */
    OWLQualifiedRestriction(OWLPropertyExpression::Ptr property, const OWLQualification& qualification)
        : OWLRestriction(property)
    {
        if(qualification == IRI())
        {
            if(isDataRestriction())
            {
                mQualification = owl_om::vocabulary::RDFS::Literal();
            } else {
                mQualification = owl_om::vocabulary::OWL::Thing();
            }
        } else {
            mQualification = qualification;
        }

        mQualified = !( mQualification == owl_om::vocabulary::OWL::Thing() || mQualification == owl_om::vocabulary::RDFS::Literal() );
    }

    /**
     * Retrieve the qualification for this restriction
     * \return Given qualification
     */
    OWLQualification getQualification() const { return mQualification; }

    /**
     * \details Determines if this restriction is qualified. Qualified cardinality restrictions are defined to be cardinality restrictions that have fillers which aren't TOP (owl:Thing or rdfs:Literal). An object restriction is unqualified if it has a filler that is owl:Thing. A data restriction is unqualified if it has a filler which is the top data type (rdfs:Literal).
     * \return true if the given qualification differs from the TOP concepts, false otherwise
     */
    bool isQualified() const
    {
        return mQualified;
    }

};

} // end namespace model
} // end namespace owlapi
#endif // OWL_API_MODEL_QUALIFIED_RESTRICTION_HPP
