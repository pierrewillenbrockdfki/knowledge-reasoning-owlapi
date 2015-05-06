#ifndef OWLAPI_MODEL_CARDINALITY_RESTRICTION_HPP
#define OWLAPI_MODEL_CARDINALITY_RESTRICTION_HPP

#include <vector>
#include <stdint.h>
#include <owlapi/model/OWLQualifiedRestriction.hpp>

namespace owlapi {
namespace model {

class OWLOntologyReader;

/**
 * \class OWLCardinalityRestriction
 */
class OWLCardinalityRestriction : public OWLQualifiedRestriction
{
    friend class OWLOntologyReader;

public:
    typedef boost::shared_ptr<OWLCardinalityRestriction> Ptr;

    enum CardinalityRestrictionType { UNKNOWN, MIN, MAX, EXACT };

    static std::map<CardinalityRestrictionType, std::string> CardinalityRestrictionTypeTxt;

    /**
     * Default constructor to allow usage of this class in a map
     */
    OWLCardinalityRestriction();

    OWLCardinalityRestriction(OWLPropertyExpression::Ptr property, uint32_t cardinality, const OWLQualification& qualification, CardinalityRestrictionType restrictionType);

    virtual ~OWLCardinalityRestriction() {}

    uint32_t getCardinality() const { return mCardinality; }

    /**
     * Get the restriction type
     */
    virtual CardinalityRestrictionType getCardinalityRestrictionType() const { return mCardinalityRestrictionType; }

    virtual std::string toString() const;

    /** 
     * Convert to exact mapping
     */
    static std::map<IRI, uint32_t> convertToExactMapping(const std::vector<OWLCardinalityRestriction::Ptr>& restrictions);

    /**
     * Set cardinality -- e.g. to allow incremental construction of restrictions
     * or reuse
     */
    void setCardinality(uint32_t cardinality) { mCardinality = cardinality; }

protected:
    void setCardinalityRestrictionType(OWLCardinalityRestriction::CardinalityRestrictionType type) { mCardinalityRestrictionType = type; }

    /**
     * Depending on the property type, narrow the internally used class to
     * either OWLObject.. or OWLData
     */
    OWLCardinalityRestriction::Ptr narrow() const;

private:
    uint32_t mCardinality;
    CardinalityRestrictionType mCardinalityRestrictionType;
};

} // end namespace model
} // end namespace owlapi

#endif // OWLAPI_MODEL_CARDINALITY_RESTRICTION_HPP
