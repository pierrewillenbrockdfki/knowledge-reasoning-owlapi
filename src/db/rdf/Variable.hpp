#ifndef OWLAPI_DB_QUERY_UNBOUND_VARIABLE_HPP
#define OWLAPI_DB_QUERY_UNBOUND_VARIABLE_HPP

#include <owlapi/OWLApi.hpp>
#include <vector>

namespace owlapi {
namespace db {
namespace query {

/**
 * Variable represent a query object. It can be either an
 * ungrounded variable such as '?s' or a prefix or fully qualified IRI
 */
class Variable : public owlapi::model::IRI
{
    /**
     * Check whether this variable requires to be quoted in a query
     */
    bool needsQuoting() const;

public:
    /**
     * Default Variable constructor
     */
    Variable();

    /**
     * Constructor to create variable from IRI
     */
    Variable(const IRI& iri);

    /**
     * Constructor to create ground or ungrounded variables
     */
    Variable(const std::string& name, bool grounded);

    /**
     * Get name of this variable, i.e. for an ungrounded variable
     * it removes the leading question mark
     */
    std::string getName() const;

    /**
     * Get query name of this variable, ungrounded variables
     * will be presented with leading question marks,
     * and URI will be quoted, e.g., <http://www.w3.org/>
     * \return Query name of this variable
     */
    std::string getQueryName() const;

    /**
     * Test if variable is grounded or just represents a placeholder
     */
    bool isGrounded() const;
};

typedef std::vector<Variable> VariableList;
typedef VariableList Bindings;


} // end namespace query
} // end namespace db
} // end namespace owlapi
#endif // OWLAPI_DB_QUERY_UNBOUND_VARIABLE_HPP
