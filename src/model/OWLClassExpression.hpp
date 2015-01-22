#ifndef OWLAPI_MODEL_OWL_CLASS_EXPRESSION_HPP
#define OWLAPI_MODEL_OWL_CLASS_EXPRESSION_HPP

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <owlapi/model/OWLObject.hpp>

namespace owlapi {
namespace model {

/**
 * \brief OWLClassExpression represent a class definition in simple (Class) or complex form, e.g. using
 * intersection definition
 *
 * \see http://www.w3.org/TR/owl2-syntax/#Class_Expression_Axioms
 */
class OWLClassExpression : public OWLObject //, OWLPropertyRange, SWRLPredicate
{
public:
    /// ClassExpressionType represents all subtypes
    /// of ClassExpressions
    enum ClassExpressionType { DATA_ALL_VALUES_FROM
        , DATA_EXACT_CARDINALITY
        , DATA_HAS_VALUE
        , DATA_MAX_CARDINALITY
        , DATA_MIN_CARDINALITY
        , DATA_SOME_VALUES_FROM
        , OBJECT_ALL_VALUES_FROM
        , OBJECT_COMPLEMENT_OF
        , OBJECT_HAS_SELF
        , OBJECT_HAS_VALUE
        , OBJECT_INTERSECTION_OF
        , OBJECT_EXACT_CARDINALITY
        , OBJECT_MAX_CARDINALITY
        , OBJECT_MIN_CARDINALITY
        , OBJECT_ONE_OF
        , OBJECT_SOME_VALUES_FROM
        , OBJECT_UNION_OF
        , OWL_CLASS
        , TYPE_RANGE_END
    };

    typedef boost::shared_ptr<OWLClassExpression> Ptr;

    static std::map<ClassExpressionType, std::string> TypeTxt;

    /**
     * \brief Gets the class expression type for this class expression
     * \return the class expression type
     */
    virtual ClassExpressionType getClassExpressionType() const { throw std::runtime_error("ClassExpression::getClassExpressionType not implemented"); }

    /**
     * \brief Determines whether or not this expression represents an anonymous class expression.
     * \return true if this is an anonymous class expression, false if this is a named class (OWLClass)
     */
    virtual bool isAnonymous() const { throw std::runtime_error("OWLClassExpression::isAnonymous: not implemented"); }

    /**
     * \brief Determines if this class is a literal. A literal being either a named class or the negation of a named class (i.e. A or not(A)).
     * \return true if this is a literal, or false if this is not a literal.
     */
    virtual bool isClassExpressionLiteral() const { throw std::runtime_error("OWLClassExpression::isClassExpressionLiteral: not implemented"); }

    /**
     * \brief Convert this object to a string -- for debugging purposes
     * \return stringified object
     */
    virtual std::string toString() const { return "ClassExpression: " + TypeTxt[getClassExpressionType()]; }
};

typedef std::vector<OWLClassExpression::Ptr> OWLClassExpressionPtrList;

} // end namespace model
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_CLASS_EXPRESSION_HPP
