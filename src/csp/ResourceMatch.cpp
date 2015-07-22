#include "ResourceMatch.hpp"
#include <algorithm>
#include <gecode/gist.hh>
#include <numeric/Combinatorics.hpp>

using namespace owlapi::model;

namespace owlapi {
namespace csp {

ResourceMatch::ResourceMatch(const std::vector<OWLCardinalityRestriction::Ptr>& queryRestrictions, const InstanceList& resources, OWLOntology::Ptr ontology)
    : mQueryRestrictions(queryRestrictions)
    , mResourcePool( resources )
    , mSetAssignment(*this, queryRestrictions.size())
{
    TypeInstanceMap query = ResourceMatch::toTypeInstanceMap(queryRestrictions);

    /// Allow mapping from the resources to a given id
    TypeInstanceMap resourcePoolTypeMap;
    {
        InstanceList::const_iterator iit = mResourcePool.begin();
        int index = 0;
        for(; iit != mResourcePool.end(); ++iit)
        {
            resourcePoolTypeMap[*iit].push_back(index++);
        }
    }

    // The restriction will be fulfilled by a set of instances -- that are
    // defined by the resourcePoolRestrictions in this case
    Gecode::IntSet allDomainValues(0, mResourcePool.size()-1);

    // assignment of a set of available resources
    // see http://www.gecode.org/doc-latest/reference/group__TaskModelSet.html
    // domain set assignement: SRT_SUB --> subset
    dom(*this, mSetAssignment, Gecode::SRT_SUB, allDomainValues);

    // Domain must not be empty
    // domain set assignment: SRT_NQ --> disequality
    dom(*this, mSetAssignment, Gecode::SRT_NQ, Gecode::IntSet(1,0));

    LOG_DEBUG_S << "All domain values: " << allDomainValues;

    // Make sure the assignments are unique, i.e. pairwise disjoint sets
    std::vector<int> indexes;
    for(int i = 0; i < mSetAssignment.size(); ++i)
    {
        indexes.push_back(i);
    }

    // If there is just one entry -- no need for computing disjoint relationship
    if(indexes.size() > 1)
    {
        numeric::Combination<int> combination(indexes, 2, numeric::EXACT);
        do {
            std::vector<int> indexes = combination.current();
            // SRT_DISJ --> disjoint
            rel(*this, mSetAssignment[ indexes[0] ], Gecode::SRT_DISJ, mSetAssignment[ indexes[1] ]);
        } while(combination.next());
    }


    //// For all resource types in the pool, find all resources that are allowed
    //// to fulfill the query
    AllowedTypesMap allowedTypesMap = ResourceMatch::getAllowedTypes(query, resourcePoolTypeMap, ontology);

    uint32_t assignmentIndex = 0;
    std::vector<OWLCardinalityRestriction::Ptr>::const_iterator cit = queryRestrictions.begin();
    for(; cit != queryRestrictions.end(); ++cit)
    {
        OWLCardinalityRestriction::Ptr restriction = *cit;

        // Compute allowedPoolResources for this slot
        std::vector<int> domain = getAllowedDomain(restriction->getQualification(), allowedTypesMap, resourcePoolTypeMap);
        // initialize with an array int[] and given size n, does not really work
        // as expected, create only a range [0,domain.size()] -- Gecode::IntSet set(domain[0], domain.size());
        Gecode::IntArgs args;
        std::vector<int>::const_iterator dit = domain.begin();
        for(; dit != domain.end(); ++dit)
        {
            args << *dit;
        }
        Gecode::IntSet allowedPoolResources(args);

        // Limit the domain of the current slot (restriction) to the allowed
        // domain values
        dom(*this, mSetAssignment[assignmentIndex], Gecode::SRT_SUB, allowedPoolResources);

        // Set the cardinality for each slot (restriction)
        switch(restriction->getCardinalityRestrictionType())
        {
            case OWLCardinalityRestriction::MIN:
                // Increasing the upper limit can lead to a significant
                // performance impact
                cardinality(*this, mSetAssignment[assignmentIndex], restriction->getCardinality(),  restriction->getCardinality());// Gecode::Set::Limits::max);
                break;
            case OWLCardinalityRestriction::MAX:
                cardinality(*this, mSetAssignment[assignmentIndex], 0, restriction->getCardinality());
                break;
            case OWLCardinalityRestriction::EXACT:
                cardinality(*this, mSetAssignment[assignmentIndex], restriction->getCardinality(), restriction->getCardinality());
                break;
            default:
                throw std::runtime_error("ResourceMatch: internal error -- unknown OWLCardinalityRestrictionType provided");
        }

        assignmentIndex++;
    }

    // Set branchers -- since otherwise we will have no assigned solutions
    branch(*this, mSetAssignment, Gecode::SET_VAR_MIN_MAX(), Gecode::SET_VAL_MAX_INC());
    branch(*this, mSetAssignment, Gecode::SET_VAR_RND( Gecode::Rnd(0)), Gecode::SET_VAL_MAX_INC());

    //Gecode::Gist::Print<ResourceMatch> p("Print solution");
    //Gecode::Gist::Options o;
    //o.inspect.click(&p);
    //Gecode::Gist::bab(this, o);

}

ResourceMatch::ResourceMatch(bool share, ResourceMatch& other)
    : Gecode::Space(share, other)
    , mQueryRestrictions(other.mQueryRestrictions)
    , mResourcePool(other.mResourcePool)
{
    mSetAssignment.update(*this, share, other.mSetAssignment);
    LOG_DEBUG_S << "ResourceMatch: construct: " << toString();
}

Gecode::Space* ResourceMatch::copy(bool share)
{
    return new ResourceMatch(share,*this);
}

//void ResourceMatch::constrain(const Gecode::Space& _b)
//{
//    const ResourceMatch& b = static_cast<const ResourceMatch&>(_b);
//
//}

TypeInstanceMap ResourceMatch::toTypeInstanceMap(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& restrictions)
{
    TypeInstanceMap typeInstanceMap;
    int instanceId = 0;
    // We assume a compact representation of the query restrictions, meaning
    // no 'overlapping' restrictions
    std::vector<OWLCardinalityRestriction::Ptr>::const_iterator cit = restrictions.begin();
    for(; cit != restrictions.end(); ++cit)
    {
        std::vector<int> instances;
        OWLCardinalityRestriction::Ptr restriction = *cit;
        uint32_t cardinality = restriction->getCardinality();
        OWLQualification qualification = restriction->getQualification();

        for(uint32_t i = 0; i < cardinality; ++i)
        {
            instances.push_back(instanceId++);
        }
        typeInstanceMap[qualification] = instances;
    }
    return typeInstanceMap;
}

InstanceList ResourceMatch::getInstanceList(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& restrictions, bool useMaxCardinality)
{
    using namespace owlapi::model;

    IRIList instances;
    // Retrieve bound per qualification (only valid if restriction apply to the
    // same property)
    std::map<IRI, OWLCardinalityRestriction::MinMax> modelCount = OWLCardinalityRestriction::getBounds(restrictions);

    std::map<IRI, OWLCardinalityRestriction::MinMax>::const_iterator mit = modelCount.begin();
    for(; mit != modelCount.end(); ++mit)
    {
        const IRI& qualification = mit->first;
        const std::pair<uint32_t, uint32_t>& minMax = mit->second;

        LOG_DEBUG_S << "qualification: " << qualification.toString() << " min: " << minMax.first << ", max: " << minMax.second;

        uint32_t cardinality = minMax.first;
        // optimistic would mean infinite resources if only min is defined
        if(useMaxCardinality)
        { 
            if(minMax.second == std::numeric_limits<uint32_t>::max())
            {
                LOG_DEBUG_S << "No upper bound given for " << qualification << ", skipping for optimistic estimation of resource instances";
            } else {
                cardinality = minMax.second;
            }
        }

        for(uint32_t i = 0; i < cardinality; ++i)
        {
            instances.push_back(qualification);
        }
    }
    return instances;
}

TypeList ResourceMatch::getTypeList(const std::vector<owlapi::model::OWLCardinalityRestriction::Ptr>& restrictions)
{
    IRIList types;

    // We assume a compact representation of the query restrictions
    std::vector<OWLCardinalityRestriction::Ptr>::const_iterator cit = restrictions.begin();
    for(; cit != restrictions.end(); ++cit)
    {
        OWLCardinalityRestriction::Ptr restriction = *cit;
        OWLQualification qualification = restriction->getQualification();
        types.push_back(qualification);
    }
    return types;
}

AllowedTypesMap ResourceMatch::getAllowedTypes(const TypeInstanceMap& query, const TypeInstanceMap& pool, OWLOntology::Ptr ontology)
{
    OWLOntologyAsk ask(ontology);

    AllowedTypesMap allowedTypesMap;
    TypeInstanceMap::const_iterator cit = query.begin();
    for(; cit != query.end(); ++cit)
    {
        std::vector<IRI> allowedTypes;
        TypeInstanceMap::const_iterator rit = pool.begin();
        for(; rit != pool.end(); ++rit)
        {
            // Either items are of the same type, or a subclass of the requested
            // one
            if(cit->first == rit->first || ask.isSubclassOf(rit->first, cit->first))
            {
                allowedTypes.push_back(rit->first);
            }
        }
        allowedTypesMap[cit->first] = allowedTypes;
    }
    return allowedTypesMap;
}

std::vector<int> ResourceMatch::getAllowedDomain(const owlapi::model::IRI& item, const AllowedTypesMap& allowedTypes, const TypeInstanceMap& typeInstanceMap )
{
    /// 1. get the allowed types for the given item
    /// 2. find instances (that are part of the resource pool) that are of an allowed type
    /// --> all allowed instances that form (part of) the domain
    std::vector<int> domain;
    AllowedTypesMap::const_iterator cit = allowedTypes.find(item);
    const std::vector<IRI>& types = cit->second;

    std::vector<IRI>::const_iterator tit = types.begin();
    for(; tit != types.end(); ++tit)
    {
        TypeInstanceMap::const_iterator iit = typeInstanceMap.find(*tit);
        const std::vector<int>& instances = iit->second;
        domain.insert(domain.end(), instances.begin(), instances.end());
    }
    return domain;
}

uint32_t ResourceMatch::getInstanceCount(const TypeInstanceMap& map)
{
    uint32_t count = 0;
    TypeInstanceMap::const_iterator cit = map.begin();
    for(; cit != map.end(); ++cit)
    {
        count += cit->second.size();
    }
    return count;
}

ResourceMatch* ResourceMatch::solve(const std::vector<OWLCardinalityRestriction::Ptr>& queryRestrictions, const std::vector<OWLCardinalityRestriction::Ptr>& resourcePoolRestrictions, OWLOntology::Ptr ontology)
{
    LOG_DEBUG_S << "Solve: resource pool restrictions" << owlapi::model::OWLCardinalityRestriction::toString(resourcePoolRestrictions);
    InstanceList instanceList = getInstanceList(resourcePoolRestrictions, true);
    LOG_DEBUG_S << "Solve: instances: " << instanceList;
    LOG_DEBUG_S << "Solve: restriction: " << owlapi::model::OWLCardinalityRestriction::toString(queryRestrictions);
    ResourceMatch* match = new ResourceMatch(queryRestrictions, instanceList, ontology);
    ResourceMatch* solvedMatch = match->solve();
    delete match;
    match = NULL;

    solvedMatch->remapSolution();
    return solvedMatch;

}

ResourceMatch* ResourceMatch::solve(const std::vector<OWLCardinalityRestriction::Ptr>& queryRestrictions, const InstanceList& resourcePool, OWLOntology::Ptr ontology)
{
    ResourceMatch* match = new ResourceMatch(queryRestrictions, resourcePool, ontology);
    ResourceMatch* solvedMatch = match->solve();
    delete match;
    match = NULL;

    solvedMatch->remapSolution();
    return solvedMatch;

}


void ResourceMatch::remapSolution()
{
    // update solution vector
    InstanceList queryInstances = getTypeList(mQueryRestrictions);

    for(int i = 0; i < mSetAssignment.size(); ++i)
    {
        InstanceList assignedResources;
        for(Gecode::SetVarGlbValues v(mSetAssignment[i]); v(); ++v)
        {
            // v.val() --> the assigned value
            assignedResources.push_back(mResourcePool[v.val()]);
        }

        OWLCardinalityRestriction::Ptr restriction = mQueryRestrictions[i];
        mSolution[restriction] = assignedResources;
    }
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
    InstanceList queryInstances = getTypeList(mQueryRestrictions);

    std::stringstream ss;
    ss << "ResourceMatch: #" << mSetAssignment.size() << " { " << std::endl;
    for(int i = 0; i < mSetAssignment.size(); i++)
    {
        ss << "    " << i << " --> " << mSetAssignment[i] << std::endl;
        ss << "    " << queryInstances[i] << " := " << std::endl;
        if(mSetAssignment[i].assigned())
        {
            for(Gecode::SetVarGlbValues v(mSetAssignment[i]); v(); ++v)
            {
                ss << "            " << mResourcePool[v.val()] << std::endl;
            }
        } else {
            ss << "        "<<  "unassigned " << std::endl;
        }
        ss << ", "<< std::endl;
    }
    ss << "}" << std::endl;
    return ss.str();
}

InstanceList ResourceMatch::getAssignedResources(OWLCardinalityRestriction::Ptr restriction) const
{
    std::map<OWLCardinalityRestriction::Ptr, InstanceList>::const_iterator cit = mSolution.find(restriction);
    if(cit != mSolution.end())
    {
        return cit->second;
    } else
    {
        throw std::runtime_error("ResourceMatch::getAssignment: there exists no assignment for '" + restriction->toString() + "'");
    }
}


InstanceList ResourceMatch::getUnassignedResources() const
{
    InstanceList instances = mResourcePool;
    std::map<OWLCardinalityRestriction::Ptr, InstanceList>::const_iterator cit = mSolution.begin();
    for(; cit != mSolution.end(); ++cit)
    {
        InstanceList assignedResources = cit->second;
        InstanceList::const_iterator iit = assignedResources.begin();
        for(; iit != assignedResources.end(); ++iit)
        {
            InstanceList::iterator uit = std::find(instances.begin(), instances.end(), *iit);
            instances.erase(uit);
        }
    }
    return instances;
}

ResourceMatch::Ptr ResourceMatch::isSupporting(const owlapi::model::IRI& providerModel, const owlapi::model::IRI& serviceModel, OWLOntology::Ptr ontology)
{
    OWLOntologyAsk ask(ontology);
    std::vector<OWLCardinalityRestriction::Ptr> serviceRestrictions = ask.getCardinalityRestrictions(serviceModel);
    std::vector<OWLCardinalityRestriction::Ptr> providerRestrictions = ask.getCardinalityRestrictions(providerModel);

    try {
        ResourceMatch* fulfillment = ResourceMatch::solve(serviceRestrictions, providerRestrictions, ontology);
        return ResourceMatch::Ptr(fulfillment);
    } catch(const std::runtime_error& e)
    {
        return ResourceMatch::Ptr();
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
        try {
            ResourceMatch* fulfillment = ResourceMatch::solve(serviceRestriction, providerRestrictions, ontology);
            supportedModels.push_back(serviceModel);
            LOG_DEBUG_S << "Fulfillment for: ";
            LOG_DEBUG_S << "    service model: " << serviceModel;
            LOG_DEBUG_S << "    combination of provider models: " << combinations;
            LOG_DEBUG_S << fulfillment->toString();
            delete fulfillment;
        } catch(const std::runtime_error& e)
        {
            // not supported
        }
    }
    return supportedModels;
}

} // end namespace csp
} // end namespace owlapi
