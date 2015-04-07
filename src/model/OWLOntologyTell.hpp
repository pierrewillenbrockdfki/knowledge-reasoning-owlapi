#ifndef OWLAPI_MODEL_OWL_ONTOLOGY_TELL_HPP
#define OWLAPI_MODEL_OWL_ONTOLOGY_TELL_HPP

#include <owlapi/model/OWLOntology.hpp>
#include <owlapi/model/OWLLiteral.hpp>
#include <owlapi/model/OWLCardinalityRestriction.hpp>

namespace owlapi {
namespace model {

/**
 * This class is allowed to add new information to an ontology
 */
class OWLOntologyTell
{
    OWLOntology::Ptr mpOntology;

public:
    typedef boost::shared_ptr<OWLOntologyTell> Ptr;

    OWLOntologyTell(OWLOntology::Ptr ontology);

    /**
     * Initialize all default classes for this ontology
     */
    void initializeDefaultClasses();

    /**
     * Get or create the OWLClass instance by IRI
     * \return OWLClass::Ptr
     */
    OWLClass::Ptr klass(const IRI& iri);

    /**
     * Get or create the OWLAnonymousIndividual by IRI
     * \return OWLAnonymousIndividual::Ptr
     */
    OWLAnonymousIndividual::Ptr anonymousIndividual(const IRI& iri);

    /**
     * Get or create the OWLNamedIndividual by IRI
     * \return OWLNamedIndividual::Ptr
     */
    OWLNamedIndividual::Ptr namedIndividual(const IRI& iri);

    /**
     * Get or create the OWLObjectProperty by IRI
     * \return OWLObjectProperty::Ptr
     */
    OWLObjectProperty::Ptr objectProperty(const IRI& iri);

    /**
     * Get or create the OWLDataProperty by IRI
     * \return OWLDataProperty::Ptr
     */
    OWLDataProperty::Ptr dataProperty(const IRI& iri);

    OWLSubClassOfAxiom::Ptr subclassOf(OWLClass::Ptr subclass, OWLClass::Ptr superclass);
    OWLSubClassOfAxiom::Ptr subclassOf(const IRI& subclass, OWLClassExpression::Ptr superclass);

    /**
     * Declare the subclass relationship between two classes
     */
    OWLSubClassOfAxiom::Ptr subclassOf(const IRI& subclass, const IRI& superclass);

    /**
     * Declare the subclass relationship between two classes
     */
    OWLSubClassOfAxiom::Ptr subclassOf(OWLClassExpression::Ptr subclassExpression, OWLClassExpression::Ptr superclassExpression);

    /**
     * Set instance of class type
     */
    OWLClassAssertionAxiom::Ptr instanceOf(const IRI& instance, const IRI& classType);

    /**
     * Create a cardinality restriction (for consistency reasons)
     * \see OWLCardinalityRestriction
     * \return restriction
     */
    OWLCardinalityRestriction::Ptr cardinalityRestriction(OWLPropertyExpression::Ptr property, uint32_t cardinality, const OWLQualification& qualification, OWLCardinalityRestriction::CardinalityRestrictionType restrictionType);

    /**
     * Add an axiom
     * \param axiom
     */
    void addAxiom(OWLAxiom::Ptr axiom);

    void inverseFunctionalProperty(const IRI& property);
    void transitiveProperty(const IRI& property);
    void functionalObjectProperty(const IRI& property);
    void functionalDataProperty(const IRI& property);
    void relatedTo(const IRI& subject, const IRI& relation, const IRI& object);
    void dataPropertyDomainOf(const IRI& relation, const IRI& classType);
    void objectPropertyDomainOf(const IRI& relation, const IRI& classType);
    void objectPropertyRangeOf(const IRI& relation, const IRI& classType);
    void inverseOf(const IRI& relation, const IRI& inverseType);

    // DataPropertyAssert
    void valueOf(const IRI& instance, const IRI& dataProperty, OWLLiteral::Ptr literal);
    void restrictClass(const IRI& klass, OWLCardinalityRestriction::Ptr restriction);

};

} // end namespace model
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_ONTOLOGY_TELL_HPP
