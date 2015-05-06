#ifndef OWLAPI_MODEL_OWL_RESTRICTION_HPP
#define OWLAPI_MODEL_OWL_RESTRICTION_HPP

#include <owlapi/model/OWLClassExpression.hpp>
#include <owlapi/model/OWLPropertyExpression.hpp>

namespace owlapi {
namespace model {

/**
 * \class OWLRestriction
 * \brief Represents a general restriction
 * \see http://owlapi.sourceforge.net/javadoc/index.html
 */
class OWLRestriction : public OWLClassExpression
{
    OWLPropertyExpression::Ptr mpProperty;

public: 
    typedef boost::shared_ptr<OWLRestriction> Ptr;

    OWLRestriction(OWLPropertyExpression::Ptr property)
        : OWLClassExpression()
        , mpProperty(property)
    {}

    virtual ~OWLRestriction() {}

    virtual OWLPropertyExpression::Ptr getProperty() const { return mpProperty; }

    /**
     * Set property
     */
    void setProperty(OWLPropertyExpression::Ptr property) { mpProperty = property; }

    virtual bool isDataRestriction() const;
    virtual bool isObjectRestriction() const;

    virtual std::string toString() const;
};

typedef std::vector<OWLRestriction::Ptr> OWLRestrictionPtrList;

} // end namespace model
} // end namespace owlapi

/**
 * Stream operator for writing cardinality restriction lists
 */
std::ostream& operator<<(std::ostream& os, const owlapi::model::OWLRestrictionPtrList& list);

#endif // OWLAPI_MODEL_OWL_RESTRICTION_HPP
