#ifndef OWLAPI_MODEL_OWL_DATATYPE_HPP
#define OWLAPI_MODEL_OWL_DATATYPE_HPP

#include "OWLDataRange.hpp"
#include "OWLLogicalEntity.hpp"

namespace owlapi {
namespace model {

class OWLDataType : public OWLDataRange, public OWLLogicalEntity
{
public:
    typedef shared_ptr<OWLDataType> Ptr;

    OWLDataType(const IRI& iri, OWLDataRange::Type type = OWLDataRange::DATATYPE);
    virtual ~OWLDataType() = default;

    /**
     * Pick the datatype for a given value, by picking from the options defined
     * by the data range
     * \param range OWLDataRange include DATATYPE, ONE_OF, ...
     */
    static OWLDataType fromRange(const OWLDataRange::Ptr& range, const std::string& value = "");

    // OWL2DataType getBuiltInDataType() const;

    bool isBoolean() const;
    bool isDouble() const;
    bool isFloat() const;
    bool isInteger() const;
    bool isRDFPlainLiteral() const;
    bool isString() const;
};

} // end namespace model
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_DATATYPE_HPP
