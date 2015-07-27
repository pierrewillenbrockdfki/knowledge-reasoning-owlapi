#ifndef OWLAPI_CSP_RESOURCE_MATCH_HPP
#define OWLAPI_CSP_RESOURCE_MATCH_HPP

#include <gecode/set.hh>
#include <gecode/int.hh>
#include <gecode/search.hh>

#include <owlapi/model/OWLOntology.hpp>
#include <owlapi/model/OWLOntologyAsk.hpp>
#include <owlapi/csp/ModelBound.hpp>
#include <base/Logging.hpp>

namespace owlapi {
namespace csp {

typedef std::map<owlapi::model::IRI, std::vector<int> > TypeInstanceMap;
typedef std::map<owlapi::model::IRI, owlapi::model::IRIList > AllowedTypesMap;
typedef owlapi::model::IRIList InstanceList;
typedef owlapi::model::IRIList TypeList;

/**
 * ResourceMatch allows to search for a valid solution for checking cardinality constraints
 * of resources by formulation as CSP (Constraint Satisfaction Problem).
 * The CSP is defined by a query that searches to fulfill a set of cardinality
 * restrictions. Available resources are implicitly defined -- also by a list of
 * cardinality restrictions. This originates from the fact that these
 * restrictions describe a model that a certain instance ('robot') fulfills and thus
 * defines which resource have to exist for this robot.
 *
 * While in an actual instance resources might be missing solving the csp allows for model
 * checking in the first place.
 *
 \verbatim

 using namespace owlapi;

 OWLOntology::Ptr ontology = reader.fromFile("organization-model.owl");
 ...
 IRI sherpa = owlapi::vocabulary::OM::resolve("Sherpa");
 IRI move_to = owlapi::vocabulary::OM::resolve("MoveTo");

 std::vector<OWLCardinalityRestriction::Ptr> r_sherpa = ask.getCardinalityRestrictions(sherpa);
 std::vector<OWLCardinalityRestriction::Ptr> r_move_to = ask.getCardinalityRestrictions(move_to);

 try {
     // Check whether 'move_to' provides a subset of restriction existing for 'sherpa'
     csp::ResourceMatch* fulfillment = cs::ResourceMatch::solve(r_move_to, r_sherpa, ontology);
     std::cout << fulfillment->toString() << std::endl;
     ...
     delete fulfillment;
 } catch(const std::runtime_error& e)
 {
     std::cout << "No solution found" << std::endl;
 }

 // or in compact form
 ResourceMatch::Ptr fulfillment = csp::ResourceMatch::fulfills(sherpa, move_to, ontology);
 if(fulfillment)
 {
    std::cout << fulfillment->toString() << std::endl;
 }
 \endverbatim
 */
class ResourceMatch : public Gecode::Space
{
    ModelBound::List mRequiredModelBound;
    ModelBound::List mAvailableModelBound;

    /**
     * Assignments of query resources to pool resources. This is what has to be solved.
     *                    available-service-0 available-service1 
     * requirement
     * service-0                  1                   2            
     * service-1                  1
     *
     * Constraints apply to min/max of sum of row --> e.g. min/max cardinality of service-0
     * Constraints apply to min/max of sum of col --> e.g. min/max cardinality of available-service-0
     * Constraints apply to individual cells due to fulfillment of service (we assume most granular resolution of services)
     */
    Gecode::IntVarArray mModelAssignment;

    ResourceMatch* solve();

protected:
    ResourceMatch(const ModelBound::List& required,
            const ModelBound::List& provided,
            owlapi::model::OWLOntology::Ptr ontology);

    /**
     * Search support
     * This copy constructor is required for the search engine
     * and it has to provide a deep copy
     */
    ResourceMatch(bool share, ResourceMatch& s);

    /**
     * This method is called by the search engine
     */
    virtual Gecode::Space* copy(bool share);

public:
    typedef boost::shared_ptr<ResourceMatch> Ptr;
    struct Solution
    {
        ModelBound::List modelBounds;
        std::string toString() const
        {
            return ModelBound::toString(modelBounds);
        }
    };

    ResourceMatch::Solution getSolution() const;

    void print(std::ostream& os) const;

    static ResourceMatch::Solution solve(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& modelRequirements,
            const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& providerResources,
            owlapi::model::OWLOntology::Ptr ontology);

    static ResourceMatch::Solution solve(const ModelBound::List& required,
            const ModelBound::List& available,
            owlapi::model::OWLOntology::Ptr ontology);

    /**
     * Create a string representation of this object
     * \return Stringified ResourceMatch object
     */
    std::string toString() const;

    static ModelBound::List toModelBoundList(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& restrictions);

    /**
     * Check if the serviceModel is supported by the providerModel
     * \param providerModel
     * \param serviceModel
     * \param resourceMatch
     * \return True if the providerModel supports the serviceModel
     */
    static bool isSupporting(const owlapi::model::IRI& providerModel, const owlapi::model::IRI& serviceModel,
            owlapi::model::OWLOntology::Ptr ontology);


    static bool isSupporting(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& providerRestrictions,
            const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& serviceRestrictions,
            owlapi::model::OWLOntology::Ptr ontology);
    /**
     * Compute for a given set of model and possible models, the available
     * set of supported models, i.e., fulfilling the restrictions
     * \return List of provided services
     */
    static owlapi::model::IRIList filterSupportedModels(const owlapi::model::IRIList& combination,
            const owlapi::model::IRIList& serviceModels, owlapi::model::OWLOntology::Ptr ontology);
};

} // end namespace cps
} // end namespace owlapi
#endif // OWLAPI_CSP_RESOURCE_MATCH_HPP
