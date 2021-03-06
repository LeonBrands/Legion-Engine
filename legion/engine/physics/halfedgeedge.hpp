#pragma once
#include <core/core.hpp>
#include <physics/data/edge_label.hpp>
#include <physics/halfedgeface.hpp>

namespace legion::physics
{
	struct HalfEdgeEdge
	{
		HalfEdgeEdge* pairingEdge = nullptr;
		HalfEdgeEdge * nextEdge = nullptr;
		HalfEdgeEdge* prevEdge = nullptr;

		HalfEdgeFace* face = nullptr;

        EdgeLabel label;
        
		math::vec3 edgePosition;
        std::string id;

        HalfEdgeEdge() = default;

		HalfEdgeEdge(math::vec3 newEdgePositionPtr) : edgePosition{ newEdgePositionPtr }
		{

		}

		/**@brief sets 'nextEdge' and 'prevEdge' with the given HalfEdgeEdges
		*/
		void setNextAndPrevEdge(HalfEdgeEdge* newPrevEdge,HalfEdgeEdge* newNextEdge)
		{
			nextEdge = newNextEdge;
			prevEdge = newPrevEdge;
		}

        /**@brief Sets the pairingEdge to the passed in edge, and sets the edge.pairingEdge to this
         * if this edge or the passed edge already have pairing edge, the pairing edge of the pairing edge will be set to nullptr
         */
        void setPairingEdge(HalfEdgeEdge* edge)
        {
            pairingEdge = edge;
            edge->pairingEdge = this;
        }

		math::vec3 getLocalNormal() const
		{
			return face->normal;
		}

		/**@brief Gets the direction of the edge by getting the
		* vector starting from the current edge's position to the next edge
		*/
		math::vec3 getLocalEdgeDirection()  const
		{
			return nextEdge->edgePosition - edgePosition;
		}
	};
}

