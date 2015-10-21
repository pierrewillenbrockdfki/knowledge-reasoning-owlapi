#include "ResourceMatch.hpp"
#include <algorithm>
#include <gecode/minimodel.hh>
#include <gecode/gist.hh>
#include <numeric/Combinatorics.hpp>

using namespace owlapi::model;

namespace owlapi {
namespace csp {

std::string ResourceMatch::Solution::toString(uint32_t indent) const
{
    std::string hspace(indent,' ');
    std::stringstream ss;
    ss << hspace << "Solution:" << std::endl;
    std::map<ModelBound, ModelBound::List>::const_iterator cit = mAssignments.begin();
    for(; cit != mAssignments.end(); ++cit)
    {
        ss << hspace << "    requirement: " << std::endl;
        ss << cit->first.toString(indent + 8) << std::endl;
        ss << hspace << "    assignment: " << std::endl;
        ss << ModelBound::toString(cit->second, indent + 8) << std::endl;
    }

    return ss.str();
}

ModelBound::List ResourceMatch::Solution::getAssignments(const owlapi::model::IRI& model) const
{
    std::map<ModelBound, ModelBound::List>::const_iterator cit = mAssignments.begin();
    for(;cit != mAssignments.end(); ++cit)
    {
        if(cit->first.model == model)
        {
            return cit->second;
        }
    }

    throw std::invalid_argument("owlapi::csp::Resource::Solution::getAssignments: no assignments for model '"
            + model.toString() + "' in solution");
}

ModelBound::List ResourceMatch::Solution::substractMinFrom(const ModelBound::List& availableResources) const
{
    ModelBound::List remainingResources = availableResources;

    std::map<ModelBound, ModelBound::List>::const_iterator cit = mAssignments.begin();
    for(; cit != mAssignments.end(); ++cit)
    {
        ModelBound::List assignedModels = cit->second;

        ModelBound::List::iterator ait = assignedModels.begin();
        for(; ait != assignedModels.end(); ++ait)
        {
            const ModelBound& assignedModelBound = *ait;

            // Remove assigned modelBound from remaining resources
            ModelBound::List::iterator it = std::find_if(remainingResources.begin(), remainingResources.end(),
                    [&assignedModelBound](const ModelBound& modelBound)
                {
                    return assignedModelBound.model == modelBound.model;
                });

            if(it == remainingResources.end())
            {
                throw std::invalid_argument("owlapi::csp::ResourceMatch::Solution::substractMinFrom: could not find model '" + assignedModelBound.model.toString() + "'");
            }

            ModelBound delta = it->substractMin(assignedModelBound);

            it->min = delta.min;
            it->max = delta.max;
        }
    }
    return remainingResources;
}

ResourceMatch::ResourceMatch(const ModelBound::List& required,
        const ModelBound::List& available,
        OWLOntology::Ptr ontology)
    : mRequiredModelBound(required)
    , mAvailableModelBound(available)
    , mModelAssignment(*this, /*width --> col*/ available.size()*
            /*height --> row*/ required.size(),0,
            std::max(ModelBound::getMaxResourceCount(required),
                ModelBound::getMaxResourceCount(available)))
{
    owlapi::model::OWLOntologyAsk ask(ontology);
    Gecode::Matrix<Gecode::IntVarArray> modelAssignment(mModelAssignment,
            /*width --> col*/ available.size(), /* height --> row*/ required.size());

    for(size_t ri = 0; ri < mRequiredModelBound.size(); ++ri)
    {
        const ModelBound& requiredModelBound = mRequiredModelBound[ri];
        const owlapi::model::IRI& requiredModel = requiredModelBound.model;

        LOG_DEBUG_S << "Required model: " << requiredModel;

        Gecode::IntVarArgs args;
        for(size_t ai = 0; ai < mAvailableModelBound.size(); ++ai)
        {
            Gecode::IntVar m = modelAssignment(ai, ri);
            args << m;

            // lower bound is always 0
            rel(*this, m, Gecode::IRT_GQ, 0);

            // upper bound
            // check if the available model supports the required, i.e. is
            // either class or subclass
            // if so, then use max required value as upper bound
            const ModelBound& availableModelBound = mAvailableModelBound[ai];
            const owlapi::model::IRI& availableModel = availableModelBound.model;
            if(requiredModel == availableModel || ask.isSubClassOf(availableModel, requiredModel))
            {
                LOG_DEBUG_S << "Available model to fulfill '" << requiredModel << std::endl
                    << "    " << availableModel;
                rel(*this, m, Gecode::IRT_LQ, requiredModelBound.max);
                rel(*this, m, Gecode::IRT_LQ, availableModelBound.max);
            } else {
                // does not fulfill the requirement so force to 0
                // i.e. no support by this available model
                rel(*this, m, Gecode::IRT_LQ, 0);
            }
        }
        LOG_DEBUG_S << "Required instances of model: " << requiredModel << ", minimum: " << requiredModelBound.min;
        // Row requires a minimum of resources to fulfill the requirement
        rel(*this, sum(args) >= requiredModelBound.min);
    }

    for(size_t ai = 0; ai < mAvailableModelBound.size(); ++ai)
    {
        const ModelBound& availableModelBound = mAvailableModelBound[ai];
        Gecode::IntVarArgs args;
        for(size_t ri = 0; ri < mRequiredModelBound.size(); ++ri)
        {
            Gecode::IntVar m = modelAssignment(ai, ri);
            args << m;
        }
        rel(*this, sum(args) <= availableModelBound.max);
    }

    // Set branchers -- since otherwise we will have no assigned solutions
    branch(*this, mModelAssignment, Gecode::INT_VAR_SIZE_MAX(), Gecode::INT_VAL_SPLIT_MIN());
    branch(*this, mModelAssignment, Gecode::INT_VAR_MIN_MIN(), Gecode::INT_VAL_SPLIT_MIN());
    branch(*this, mModelAssignment, Gecode::INT_VAR_NONE(), Gecode::INT_VAL_SPLIT_MIN());

    //Gecode::Gist::Print<ResourceMatch> p("Print solution");
    //Gecode::Gist::Options o;
    //o.inspect.click(&p);
    //Gecode::Gist::bab(this, o);

}

ResourceMatch::ResourceMatch(bool share, ResourceMatch& other)
    : Gecode::Space(share, other)
    , mRequiredModelBound(other.mRequiredModelBound)
    , mAvailableModelBound(other.mAvailableModelBound)
{
    mModelAssignment.update(*this, share, other.mModelAssignment);
}

Gecode::Space* ResourceMatch::copy(bool share)
{
    return new ResourceMatch(share,*this);
}

ResourceMatch::Solution ResourceMatch::solve(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& modelRequirements,
        const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& providerResources,
        OWLOntology::Ptr ontology)
{
    ModelBound::List required = toModelBoundList(modelRequirements);
    ModelBound::List available = toModelBoundList(providerResources);

    return solve(required, available, ontology);
}

ResourceMatch::Solution ResourceMatch::solve(const ModelBound::List& required, const ModelBound::List& available, OWLOntology::Ptr ontology)
{
    LOG_INFO_S << "Solve:" << std::endl
        << "    required: " << std::endl
        << "    " << ModelBound::toString(required, 8) << std::endl
        << "    available: " << std::endl
        << "    " << ModelBound::toString(available, 8) << std::endl;

    ResourceMatch* match = new ResourceMatch(required, available, ontology);
    ResourceMatch* solvedMatch = match->solve();
    delete match;
    match = NULL;

    Solution solution = solvedMatch->getSolution();
    delete solvedMatch;
    solvedMatch = NULL;
    return solution;
}

ResourceMatch::Solution ResourceMatch::getSolution() const
{
    Gecode::Matrix<Gecode::IntVarArray> modelAssignment(mModelAssignment, mAvailableModelBound.size(), mRequiredModelBound.size());

    Solution solution;
    for(size_t mi = 0; mi < mAvailableModelBound.size(); ++mi)
    {
        // Check if resource requirements holds
        for(size_t i = 0; i < mRequiredModelBound.size(); ++i)
        {

            Gecode::IntVar var = modelAssignment(mi, i);
            if(!var.assigned())
            {
                throw std::runtime_error("owlapi::csp::ResourceMatch::getSolution: value has not been assigned");
            }

            Gecode::IntVarValues v( var );

            ModelBound modelBound(mAvailableModelBound[mi].model, v.val(), v.val());
            if(modelBound.min != 0)
            {
                solution.addAssignment(mRequiredModelBound[i], modelBound);
            }
        }

    }
    return solution;
}


void ResourceMatch::print(std::ostream& os) const
{
    os << this->toString() << std::endl;
}


ResourceMatch* ResourceMatch::solve()
{
    // Setup Branch and bound search
    // alternative is depth first search
    Gecode::BAB<ResourceMatch> searchEngine(this);
    //Gecode::DFS<ResourceMatch> searchEngine(this);

    // Start search
    ResourceMatch* best = NULL;
    while(ResourceMatch* matching = searchEngine.next())
    {
        delete best;
        // Save current solution as best
        best = matching;

        //We do not need optimization of the result,
        //just trying to validate sufficient condition,
        //so take first solution
        break;
    }

    if(best == NULL)
    {
        throw std::runtime_error("owlapi::csp::ResourceMatch: no solution found");
    }

    return best;
}

std::string ResourceMatch::toString() const
{
    std::stringstream ss;
    ss << "ResourceMatch: #" << mModelAssignment;
    return ss.str();
}

ModelBound::List ResourceMatch::toModelBoundList(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& restrictions)
{
    using namespace owlapi::model;
    std::map<owlapi::model::IRI, OWLCardinalityRestriction::MinMax> bounds = OWLCardinalityRestriction::getBounds(restrictions);

    ModelBound::List modelBounds;
    std::map<owlapi::model::IRI, OWLCardinalityRestriction::MinMax>::const_iterator cit = bounds.begin();
    for(; cit != bounds.end(); ++cit)
    {
        modelBounds.push_back( ModelBound(cit->first, cit->second.first, cit->second.second) );
    }

    return modelBounds;
}

bool ResourceMatch::isSupporting(const owlapi::model::IRI& providerModel, const owlapi::model::IRI& serviceModel, OWLOntology::Ptr ontology)
{
    OWLOntologyAsk ask(ontology);

    std::vector<OWLCardinalityRestriction::Ptr> providerRestrictions = ask.getCardinalityRestrictions(providerModel);
    std::vector<OWLCardinalityRestriction::Ptr> serviceRestrictions = ask.getCardinalityRestrictions(serviceModel);

    return isSupporting(providerRestrictions, serviceRestrictions, ontology);
}

bool ResourceMatch::isSupporting(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& providerRestrictions,
        const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& serviceRestrictions,
        owlapi::model::OWLOntology::Ptr ontology)
{
    try {
        ResourceMatch::Solution fulfillment = ResourceMatch::solve(serviceRestrictions, providerRestrictions, ontology);
        return true;
    } catch(const std::runtime_error& e)
    {
        return false;
    }
}

owlapi::model::IRIList ResourceMatch::filterSupportedModels(const owlapi::model::IRIList& combinations,
        const owlapi::model::IRIList& serviceModels, owlapi::model::OWLOntology::Ptr ontology)
{
    OWLOntologyAsk ask(ontology);
    std::vector<OWLCardinalityRestriction::Ptr> providerRestrictions = ask.getCardinalityRestrictions(combinations);
    owlapi::model::IRIList supportedModels;

    owlapi::model::IRIList::const_iterator it = serviceModels.begin();
    for(; it != serviceModels.end(); ++it)
    {
        owlapi::model::IRI serviceModel = *it;
        std::vector<OWLCardinalityRestriction::Ptr> serviceRestriction = ask.getCardinalityRestrictions(serviceModel);

        LOG_DEBUG_S << "Checking required models for '" << serviceModel.toString() << "'" << std::endl
            << OWLCardinalityRestriction::toString(serviceRestriction) << std::endl
            << " vs provided from '" << combinations << "' : "
            << OWLCardinalityRestriction::toString(providerRestrictions);

        if( ResourceMatch::isSupporting(providerRestrictions, serviceRestriction, ontology))
        {
            supportedModels.push_back(serviceModel);

            LOG_DEBUG_S << "Fulfillment for: " << std::endl
                << "    service model: " << serviceModel << std::endl
                << "    combination of provider models: " << combinations << std::endl;
        }
    }
    return supportedModels;
}

} // end namespace csp
} // end namespace owlapi
