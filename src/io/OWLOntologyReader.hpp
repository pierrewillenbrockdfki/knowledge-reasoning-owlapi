#ifndef OWLAPI_MODEL_OWL_ONTOLOGY_READER_HPP
#define OWLAPI_MODEL_OWL_ONTOLOGY_READER_HPP

#include <owlapi/model/OWLOntology.hpp>
#include <owlapi/db/rdf/SparqlInterface.hpp>

namespace owlapi {
namespace model { 

class OWLOntologyTell;
class OWLOntologyAsk;

}

namespace io {

/**
 *
 * \see http://www.w3.org/TR/owl2-syntax/#Entity_Declarations_and_Typing
 */
class OWLOntologyReader
{
    /// Pointer to the underlying query interfaces for SPARQL
    db::query::SparqlInterface* mSparqlInterface;

    std::vector<owlapi::model::IRI> mRestrictions;
    std::map<owlapi::model::IRI, std::vector<owlapi::model::OWLClass::Ptr> > mAnonymousRestrictions;
    std::map<owlapi::model::IRI, owlapi::model::OWLCardinalityRestriction> mCardinalityRestrictions;
    std::map<owlapi::model::IRI, owlapi::model::OWLValueRestriction> mValueRestrictions;

    /// The currently opened path
    std::string mAbsolutePath;

protected:

    db::query::Results findAll(const db::query::Variable& subject, const db::query::Variable& predicate, const db::query::Variable& object) const;

    void load(owlapi::model::OWLOntology::Ptr& ontology);


    void loadProperties(owlapi::model::OWLOntology::Ptr& ontology);
    void loadRestrictions(owlapi::model::OWLOntology::Ptr& ontology);
    void loadObjectProperties(owlapi::model::OWLOntology::Ptr& ontology);
    void loadDataProperties(owlapi::model::OWLOntology::Ptr& ontology);

public:
    /**
     * Open file to load ontology in an incremental way
     */
    owlapi::model::OWLOntology::Ptr open(const std::string& filename);

    /**
     * Load declarations and imports for this ontology
     * \param ontology
     * \param directImport Set to true if this a direct import false otherwise
     */
    void loadDeclarationsAndImports(owlapi::model::OWLOntology::Ptr& ontology, bool directImports);

    /**
     * Load 'the rest' after declarations and import definitions
     */
    void loadAxioms(owlapi::model::OWLOntology::Ptr& ontology);

    /**
     * Default constructor for the OWLOntologyReader
     * use the SPARQL query interface by default
     */
    OWLOntologyReader();

    virtual ~OWLOntologyReader();

    /**
     * Load ontology in one go -- only possible if no imports are used
     */
    virtual owlapi::model::OWLOntology::Ptr fromFile(const std::string& filename);

    /**
     * Retrieve the absolute path this ontology has been loaded from
     * \return path
     */
    const std::string& getAbsolutePath() const { return mAbsolutePath; }
};

} // end namespace io
} // end namespace owlapi
#endif // OWLAPI_MODEL_OWL_ONTOLOGY_READER_HPP
