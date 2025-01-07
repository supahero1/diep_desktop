#include <DiepDesktop/server/quadtree.h>
#include <DiepDesktop/shared/debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NDEBUG
	#include <DiepDesktop/shared/alloc_ext.h>

	#define QT_MALLOC(size) alloc_malloc(size)
	#define QT_CALLOC(size) alloc_calloc(size)
	#define QT_REMALLOC(old_size, ptr, new_size) alloc_remalloc(old_size, ptr, new_size)
	#define QT_FREE(size, ptr) alloc_free(size, ptr)
#else
	#define QT_MALLOC(size) malloc(size)
	#define QT_CALLOC(size) calloc(1, size)
	#define QT_REMALLOC(old_size, ptr, new_size) realloc(ptr, new_size)
	#define QT_FREE(size, ptr) free(ptr)
#endif


void
QuadtreeInit(
	Quadtree* QT
	)
{
	QT->Nodes = QT_MALLOC(sizeof(*QT->Nodes));
	QT->NodeEntities = NULL;
	QT->Entities = NULL;
#if QUADTREE_DEDUPE_COLLISIONS == 1
	QT->HTEntries = NULL;
#endif
	QT->Removals = NULL;
	QT->NodeRemovals = NULL;
	QT->Insertions = NULL;
	QT->Reinsertions = NULL;

	QT->NodesUsed = 1;
	QT->NodesSize = 1;

	QT->NodeEntitiesUsed = 1;
	QT->NodeEntitiesSize = 0;

	QT->EntitiesUsed = 1;
	QT->EntitiesSize = 0;

#if QUADTREE_DEDUPE_COLLISIONS == 1
	QT->HTEntriesSize = 0;
#endif

	QT->RemovalsUsed = 0;
	QT->RemovalsSize = 0;

	QT->NodeRemovalsUsed = 0;
	QT->NodeRemovalsSize = 0;

	QT->InsertionsUsed = 0;
	QT->InsertionsSize = 0;

	QT->ReinsertionsUsed = 0;
	QT->ReinsertionsSize = 0;

	assert_not_null(QT->Nodes);

	QT->Nodes[0].head = 0;
	QT->Nodes[0].count = 0;
	QT->Nodes[0].PositionFlags = 0b1111; /* TRBL */
}


void
QuadtreeFree(
	Quadtree* QT
	)
{
	QT_FREE(sizeof(*QT->Nodes) * QT->NodesSize, QT->Nodes);
	QT_FREE(sizeof(*QT->NodeEntities) * QT->NodeEntitiesSize, QT->NodeEntities);
	QT_FREE(sizeof(*QT->Entities) * QT->EntitiesSize, QT->Entities);
#if QUADTREE_DEDUPE_COLLISIONS == 1
	QT_FREE(sizeof(*QT->HTEntries) * QT->HTEntriesSize, QT->HTEntries);
#endif
	QT_FREE(sizeof(*QT->Removals) * QT->RemovalsSize, QT->Removals);
	QT_FREE(sizeof(*QT->NodeRemovals) * QT->NodeRemovalsSize, QT->NodeRemovals);
	QT_FREE(sizeof(*QT->Insertions) * QT->InsertionsSize, QT->Insertions);
	QT_FREE(sizeof(*QT->Reinsertions) * QT->ReinsertionsSize, QT->Reinsertions);
}


#define QuadtreeDescend(_Extent)											\
do																			\
{																			\
	QuadtreePosition HalfW = info.Extent.w * 0.5f;							\
	QuadtreePosition HalfH = info.Extent.h * 0.5f;							\
																			\
	if(_Extent.min_x <= info.Extent.x)										\
	{																		\
		if(_Extent.min_y <= info.Extent.y)									\
		{																	\
			*(NodeInfo++) =													\
			(QuadtreeNodeInfo)												\
			{																\
				.NodeIdx = node->Heads[0],									\
				.Extent =													\
				(QuadtreeHalfExtent)										\
				{															\
					.x = info.Extent.x - HalfW,								\
					.y = info.Extent.y - HalfH,								\
					.w = HalfW,												\
					.h = HalfH												\
				}															\
			};																\
		}																	\
		if(_Extent.max_y >= info.Extent.y)									\
		{																	\
			*(NodeInfo++) =													\
			(QuadtreeNodeInfo)												\
			{																\
				.NodeIdx = node->Heads[1],									\
				.Extent =													\
				(QuadtreeHalfExtent)										\
				{															\
					.x = info.Extent.x - HalfW,								\
					.y = info.Extent.y + HalfH,								\
					.w = HalfW,												\
					.h = HalfH												\
				}															\
			};																\
		}																	\
	}																		\
	if(_Extent.max_x >= info.Extent.x)										\
	{																		\
		if(_Extent.min_y <= info.Extent.y)									\
		{																	\
			*(NodeInfo++) =													\
			(QuadtreeNodeInfo)												\
			{																\
				.NodeIdx = node->Heads[2],									\
				.Extent =													\
				(QuadtreeHalfExtent)										\
				{															\
					.x = info.Extent.x + HalfW,								\
					.y = info.Extent.y - HalfH,								\
					.w = HalfW,												\
					.h = HalfH												\
				}															\
			};																\
		}																	\
		if(_Extent.max_y >= info.Extent.y)									\
		{																	\
			*(NodeInfo++) =													\
			(QuadtreeNodeInfo)												\
			{																\
				.NodeIdx = node->Heads[3],									\
				.Extent =													\
				(QuadtreeHalfExtent)										\
				{															\
					.x = info.Extent.x + HalfW,								\
					.y = info.Extent.y + HalfH,								\
					.w = HalfW,												\
					.h = HalfH												\
				}															\
			};																\
		}																	\
	}																		\
}																			\
while(0)


void
QuadtreeInsert(
	Quadtree* QT,
	const QuadtreeEntityData* data
	)
{
	if(QT->InsertionsUsed >= QT->InsertionsSize)
	{
		uint32_t new_size = (QT->InsertionsUsed << 1) | 1;

		QT->Insertions = QT_REMALLOC(sizeof(*QT->Insertions) * QT->InsertionsSize,
			QT->Insertions, sizeof(*QT->Insertions) * new_size);
		assert_not_null(QT->Insertions);

		QT->InsertionsSize = new_size;
	}

	uint32_t InsertionIdx = QT->InsertionsUsed++;
	QuadtreeInsertion* Insertion = QT->Insertions + InsertionIdx;

	Insertion->data = *data;
}


void
QuadtreeRemove(
	Quadtree* QT,
	uint32_t EntityIdx
	)
{
	if(QT->RemovalsUsed >= QT->RemovalsSize)
	{
		uint32_t new_size = (QT->RemovalsUsed << 1) | 1;

		QT->Removals = QT_REMALLOC(sizeof(*QT->Removals) * QT->RemovalsSize,
			QT->Removals, sizeof(*QT->Removals) * new_size);
		assert_not_null(QT->Removals);

		QT->RemovalsSize = new_size;
	}

	uint32_t RemovalIdx = QT->RemovalsUsed++;
	QuadtreeRemoval* Removal = QT->Removals + RemovalIdx;

	Removal->EntityIdx = EntityIdx;
}


void
QuadtreeNormalize(
	Quadtree* QT
	)
{
	if(!QT->InsertionsUsed && !QT->ReinsertionsUsed && !QT->RemovalsUsed && !QT->NodeRemovalsUsed)
	{
		return;
	}

	QuadtreeNode* Nodes = QT->Nodes;
	QuadtreeNodeEntity* NodeEntities = QT->NodeEntities;
	QuadtreeEntity* Entities = QT->Entities;

	uint32_t FreeNodeEntity = 0; /* Not reflected in QT->FreeNodeEntity */
	uint32_t NodeEntitiesUsed = QT->NodeEntitiesUsed;
	uint32_t NodeEntitiesSize = QT->NodeEntitiesSize;

	uint32_t FreeEntity = 0; /* Not reflected in QT->FreeEntity */
	uint32_t EntitiesUsed = QT->EntitiesUsed;
	uint32_t EntitiesSize = QT->EntitiesSize;

	QuadtreeNodeInfo NodeInfos[QUADTREE_DFS_LENGTH];
	QuadtreeNodeInfo* NodeInfo;


	/* node removals -> removals -> reinsertions -> insertions (this order is important) */


	if(QT->NodeRemovalsUsed)
	{
		/* First of all, this is safe, because if an entity is inserted, the actual insertion is delayed until this
		 * function, therefore it cannot be iterated over during the same update, so it can never be node removed.
		 *
		 * Second of all, this is necessary, because PrevNodeEntityIdx may no longer be true after insertions
		 * and removals are performed.
		 *
		 * Third of all, need to traverse the array backwards, because next indexes will get corrupted otherwise.
		 *
		 * Sketchy code, but it works after couple hours of debugging.
		 */

		QuadtreeNodeRemoval* NodeRemovals = QT->NodeRemovals;
		QuadtreeNodeRemoval* NodeRemoval = NodeRemovals + QT->NodeRemovalsUsed - 1;

		while(NodeRemoval >= NodeRemovals)
		{
			uint32_t NodeIdx = NodeRemoval->NodeIdx;
			QuadtreeNode* node = Nodes + NodeIdx;

			uint32_t NodeEntityIdx = NodeRemoval->NodeEntityIdx;
			QuadtreeNodeEntity* NodeEntity = NodeEntities + NodeEntityIdx;

			uint32_t PrevNodeEntityIdx = NodeRemoval->PrevNodeEntityIdx;
			QuadtreeNodeEntity* PrevNodeEntity = NodeEntities + PrevNodeEntityIdx;

			if(PrevNodeEntityIdx)
			{
				PrevNodeEntity->next = NodeEntity->next;
			}
			else
			{
				node->head = NodeEntity->next;
			}

			--node->count;

			NodeEntity->next = FreeNodeEntity;
			FreeNodeEntity = NodeEntityIdx;

			--NodeRemoval;
		}

		QT_FREE(sizeof(*NodeRemovals) * QT->NodeRemovalsSize, NodeRemovals);
		QT->NodeRemovals = NULL;
		QT->NodeRemovalsUsed = 0;
		QT->NodeRemovalsSize = 0;
	}


	{
		QuadtreeRemoval* Removals = QT->Removals;
		QuadtreeRemoval* Removal = Removals;
		QuadtreeRemoval* RemovalEnd = Removal + QT->RemovalsUsed;

		while(Removal != RemovalEnd)
		{
			NodeInfo = NodeInfos;

			*(NodeInfo++) =
			(QuadtreeNodeInfo)
			{
				.NodeIdx = 0,
				.Extent = QT->half_extent_t
			};

			uint32_t EntityIdx = Removal->EntityIdx;
			QuadtreeEntity* Entity = Entities + EntityIdx;
			QuadtreeRectExtent EntityExtent = QuadtreeGetEntityRectExtent(Entity);

			do
			{
				QuadtreeNodeInfo info = *(--NodeInfo);
				QuadtreeNode* node = Nodes + info.NodeIdx;

				if(node->count == -1)
				{
					QuadtreeDescend(EntityExtent);
					continue;
				}

				QuadtreeNodeEntity* PrevNodeEntity = NULL;
				uint32_t NodeEntityIdx = node->head;

				while(NodeEntityIdx)
				{
					QuadtreeNodeEntity* NodeEntity = NodeEntities + NodeEntityIdx;

					if(NodeEntity->Entity == EntityIdx)
					{
						if(PrevNodeEntity)
						{
							PrevNodeEntity->next = NodeEntity->next;
						}
						else
						{
							node->head = NodeEntity->next;
						}

						--node->count;

						NodeEntity->next = FreeNodeEntity;
						FreeNodeEntity = NodeEntityIdx;

						break;
					}

					PrevNodeEntity = NodeEntity;
					NodeEntityIdx = NodeEntity->next;
				}
			}
			while(NodeInfo != NodeInfos);

			Entity->next = FreeEntity;
			FreeEntity = EntityIdx;

			++Removal;
		}

		QT_FREE(sizeof(*Removals) * QT->RemovalsSize, Removals);
		QT->Removals = NULL;
		QT->RemovalsUsed = 0;
		QT->RemovalsSize = 0;
	}


	{
		QuadtreeReinsertion* Reinsertions = QT->Reinsertions;
		QuadtreeReinsertion* Reinsertion = Reinsertions;
		QuadtreeReinsertion* ReinsertionEnd = Reinsertion + QT->ReinsertionsUsed;

		while(Reinsertion != ReinsertionEnd)
		{
			uint32_t EntityIdx = Reinsertion->EntityIdx;
			QuadtreeEntity* Entity = Entities + EntityIdx;

			QuadtreeRectExtent EntityExtent = QuadtreeGetEntityRectExtent(Entity);

			NodeInfo = NodeInfos;

			*(NodeInfo++) =
			(QuadtreeNodeInfo)
			{
				.NodeIdx = 0,
				.Extent = QT->half_extent_t
			};

			do
			{
				QuadtreeNodeInfo info = *(--NodeInfo);
				QuadtreeNode* node = Nodes + info.NodeIdx;

				if(node->count == -1)
				{
					QuadtreeDescend(EntityExtent);
					continue;
				}

				uint32_t NodeEntityIdx = node->head;
				QuadtreeNodeEntity* NodeEntity;

				while(NodeEntityIdx)
				{
					NodeEntity = NodeEntities + NodeEntityIdx;

					if(NodeEntity->Entity == EntityIdx)
					{
						goto goto_skip;
					}

					NodeEntityIdx = NodeEntity->next;
				}

				if(FreeNodeEntity)
				{
					NodeEntityIdx = FreeNodeEntity;
					NodeEntity = NodeEntities + NodeEntityIdx;
					FreeNodeEntity = NodeEntity->next;
				}
				else
				{
					if(NodeEntitiesUsed >= NodeEntitiesSize)
					{
						uint32_t new_size = (NodeEntitiesUsed << 1) | 1;

						NodeEntities = QT_REMALLOC(sizeof(*NodeEntities) * NodeEntitiesSize,
							NodeEntities, sizeof(*NodeEntities) * new_size);
						assert_not_null(NodeEntities);

						NodeEntitiesSize = new_size;
					}

					NodeEntityIdx = NodeEntitiesUsed++;
					NodeEntity = NodeEntities + NodeEntityIdx;
				}

				NodeEntity->next = node->head;
				NodeEntity->Entity = EntityIdx;
				node->head = NodeEntityIdx;

				++node->count;

				goto_skip:;
			}
			while(NodeInfo != NodeInfos);

			++Reinsertion;
		}

		QT_FREE(sizeof(*Reinsertions) * QT->ReinsertionsSize, Reinsertions);
		QT->Reinsertions = NULL;
		QT->ReinsertionsUsed = 0;
		QT->ReinsertionsSize = 0;
	}


	{
		QuadtreeInsertion* Insertions = QT->Insertions;
		QuadtreeInsertion* Insertion = Insertions;
		QuadtreeInsertion* InsertionEnd = Insertion + QT->InsertionsUsed;

		while(Insertion != InsertionEnd)
		{
			QuadtreeEntityData* data = &Insertion->data;

			uint32_t EntityIdx;
			QuadtreeEntity* Entity;

			if(FreeEntity)
			{
				EntityIdx = FreeEntity;
				Entity = Entities + EntityIdx;
				FreeEntity = Entity->next;
			}
			else
			{
				if(EntitiesUsed >= EntitiesSize)
				{
					uint32_t new_size = (EntitiesUsed << 1) | 1;

					Entities = QT_REMALLOC(sizeof(*Entities) * EntitiesSize,
						Entities, sizeof(*Entities) * new_size);
					assert_not_null(Entities);

					EntitiesSize = new_size;
				}

				EntityIdx = EntitiesUsed++;
				Entity = Entities + EntityIdx;
			}

			Entity->data = *data;
			Entity->QueryTick = QT->QueryTick;
			Entity->UpdateTick = QT->UpdateTick;

			QuadtreeRectExtent EntityExtent = QuadtreeGetEntityRectExtent(Entity);

			NodeInfo = NodeInfos;

			*(NodeInfo++) =
			(QuadtreeNodeInfo)
			{
				.NodeIdx = 0,
				.Extent = QT->half_extent_t
			};

			do
			{
				QuadtreeNodeInfo info = *(--NodeInfo);
				QuadtreeNode* node = Nodes + info.NodeIdx;

				if(node->count == -1)
				{
					QuadtreeDescend(EntityExtent);
					continue;
				}

				uint32_t NodeEntityIdx;
				QuadtreeNodeEntity* NodeEntity;

				if(FreeNodeEntity)
				{
					NodeEntityIdx = FreeNodeEntity;
					NodeEntity = NodeEntities + NodeEntityIdx;
					FreeNodeEntity = NodeEntity->next;
				}
				else
				{
					if(NodeEntitiesUsed >= NodeEntitiesSize)
					{
						uint32_t new_size = (NodeEntitiesUsed << 1) | 1;

						NodeEntities = QT_REMALLOC(sizeof(*NodeEntities) * NodeEntitiesSize,
							NodeEntities, sizeof(*NodeEntities) * new_size);
						assert_not_null(NodeEntities);

						NodeEntitiesSize = new_size;
					}

					NodeEntityIdx = NodeEntitiesUsed++;
					NodeEntity = NodeEntities + NodeEntityIdx;
				}

				NodeEntityIdx = NodeEntitiesUsed++;
				NodeEntity = NodeEntities + NodeEntityIdx;

				NodeEntity->next = node->head;
				NodeEntity->Entity = EntityIdx;
				node->head = NodeEntityIdx;

				++node->count;
			}
			while(NodeInfo != NodeInfos);

			++Insertion;
		}

		QT_FREE(sizeof(*Insertions) * QT->InsertionsSize, Insertions);
		QT->Insertions = NULL;
		QT->InsertionsUsed = 0;
		QT->InsertionsSize = 0;
	}


	{
		uint32_t FreeNode = 0; /* Not reflected in QT->FreeNode */
		uint32_t NodesUsed = QT->NodesUsed;
		uint32_t NodesSize = QT->NodesSize;

		QuadtreeNode* NewNodes;
		QuadtreeNodeEntity* NewNodeEntities;
		QuadtreeEntity* NewEntities;

		uint32_t NewNodesUsed = 0;
		uint32_t NewNodesSize;

		if(NodesSize >> 2 < NodesUsed)
		{
			NewNodesSize = NodesSize;
		}
		else
		{
			NewNodesSize = NodesSize >> 1;
		}

		uint32_t NewNodeEntitiesUsed = 1;
		uint32_t NewNodeEntitiesSize;

		if(NodeEntitiesSize >> 2 < NodeEntitiesUsed)
		{
			NewNodeEntitiesSize = NodeEntitiesSize;
		}
		else
		{
			NewNodeEntitiesSize = NodeEntitiesSize >> 1;
		}

		uint32_t NewEntitiesUsed = 1;
		uint32_t NewEntitiesSize;

		if(EntitiesSize >> 2 < EntitiesUsed)
		{
			NewEntitiesSize = EntitiesSize;
		}
		else
		{
			NewEntitiesSize = EntitiesSize >> 1;
		}

		NewNodes = QT_MALLOC(sizeof(*NewNodes) * NewNodesSize);
		assert_not_null(NewNodes);

		NewNodeEntities = QT_MALLOC(sizeof(*NewNodeEntities) * NewNodeEntitiesSize);
		assert_not_null(NewNodeEntities);

		NewEntities = QT_MALLOC(sizeof(*NewEntities) * NewEntitiesSize);
		assert_not_null(NewEntities);

		uint32_t* EntityMap = QT_CALLOC(sizeof(*EntityMap) * EntitiesSize);
		assert_not_null(EntityMap);


		typedef struct QuadtreeNodeReorderInfo
		{
			uint32_t NodeIdx;
			QuadtreeHalfExtent Extent;
			uint32_t ParentNodeIdx;
			uint32_t HeadIdx;
		}
		QuadtreeNodeReorderInfo;

		QuadtreeNodeReorderInfo NodeInfos[QUADTREE_DFS_LENGTH];
		QuadtreeNodeReorderInfo* NodeInfo = NodeInfos;

		*(NodeInfo++) =
		(QuadtreeNodeReorderInfo)
		{
			.NodeIdx = 0,
			.Extent = QT->half_extent_t,
			.ParentNodeIdx = 0,
			.HeadIdx = 0
		};

		do
		{
			QuadtreeNodeReorderInfo info = *(--NodeInfo);
			QuadtreeNode* node = Nodes + info.NodeIdx;

			uint32_t NewNodeIdx = NewNodesUsed++;
			QuadtreeNode* NewNode = NewNodes + NewNodeIdx;

			NewNodes[info.ParentNodeIdx].Heads[info.HeadIdx] = NewNodeIdx;

			if(node->count == -1)
			{
				uint32_t Total = 0;
				uint32_t MaxIdx = 0;
				uint32_t MaxCount = 0;
				bool Possible = true;

				for(int i = 0; i < 4; ++i)
				{
					uint32_t NodeIdx = node->Heads[i];
					QuadtreeNode* node = Nodes + NodeIdx;

					if(node->count == -1)
					{
						Possible = false;
						break;
					}

					if(node->count > MaxCount)
					{
						MaxCount = node->count;
						MaxIdx = i;
					}

					Total += node->count;
				}

				if(!Possible || Total > QUADTREE_MERGE_THRESHOLD)
				{
					goto goto_parent;
				}

				uint32_t Heads[4];
				memcpy(Heads, node->Heads, sizeof(Heads));

				QuadtreeNode* Children[4];
				for(int i = 0; i < 4; ++i)
				{
					Children[i] = Nodes + Heads[i];
				}

				QuadtreeNode* MaxChild = Children[MaxIdx];
				node->count = MaxChild->count;
				node->head = MaxChild->head;
				node->PositionFlags = MaxChild->PositionFlags;

				for(int i = 0; i < 4; ++i)
				{
					uint32_t ChildIdx = Heads[i];
					QuadtreeNode* Child = Children[i];

					if(i == MaxIdx)
					{
						Child->next = FreeNode;
						FreeNode = ChildIdx;

						continue;
					}

					node->PositionFlags |= Child->PositionFlags;

					uint32_t NodeEntityIdx = Child->head;
					while(NodeEntityIdx)
					{
						QuadtreeNodeEntity* NodeEntity = NodeEntities + NodeEntityIdx;

						QuadtreeNodeEntity* OtherNodeEntity = NodeEntities + node->head;
						while(1)
						{
							if(NodeEntity->Entity == OtherNodeEntity->Entity)
							{
								uint32_t NextNodeEntityIdx = NodeEntity->next;

								NodeEntity->next = FreeNodeEntity;
								FreeNodeEntity = NodeEntityIdx;

								NodeEntityIdx = NextNodeEntityIdx;

								break;
							}

							if(!OtherNodeEntity->next)
							{
								OtherNodeEntity->next = NodeEntityIdx;
								NodeEntityIdx = NodeEntity->next;
								NodeEntity->next = 0;

								++node->count;

								break;
							}

							OtherNodeEntity = NodeEntities + OtherNodeEntity->next;
						}
					}

					Child->next = FreeNode;
					FreeNode = ChildIdx;
				}

				goto goto_leaf;

				goto_parent:;

				float HalfW = info.Extent.w * 0.5f;
				float HalfH = info.Extent.h * 0.5f;

				*(NodeInfo++) =
				(QuadtreeNodeReorderInfo)
				{
					.NodeIdx = node->Heads[0],
					.Extent =
					(QuadtreeHalfExtent)
					{
						.x = info.Extent.x - HalfW,
						.y = info.Extent.y - HalfH,
						.w = HalfW,
						.h = HalfH
					},
					.ParentNodeIdx = NewNodeIdx,
					.HeadIdx = 0
				};

				*(NodeInfo++) =
				(QuadtreeNodeReorderInfo)
				{
					.NodeIdx = node->Heads[1],
					.Extent =
					(QuadtreeHalfExtent)
					{
						.x = info.Extent.x - HalfW,
						.y = info.Extent.y + HalfH,
						.w = HalfW,
						.h = HalfH
					},
					.ParentNodeIdx = NewNodeIdx,
					.HeadIdx = 1
				};

				*(NodeInfo++) =
				(QuadtreeNodeReorderInfo)
				{
					.NodeIdx = node->Heads[2],
					.Extent =
					(QuadtreeHalfExtent)
					{
						.x = info.Extent.x + HalfW,
						.y = info.Extent.y - HalfH,
						.w = HalfW,
						.h = HalfH
					},
					.ParentNodeIdx = NewNodeIdx,
					.HeadIdx = 2
				};

				*(NodeInfo++) =
				(QuadtreeNodeReorderInfo)
				{
					.NodeIdx = node->Heads[3],
					.Extent =
					(QuadtreeHalfExtent)
					{
						.x = info.Extent.x + HalfW,
						.y = info.Extent.y + HalfH,
						.w = HalfW,
						.h = HalfH
					},
					.ParentNodeIdx = NewNodeIdx,
					.HeadIdx = 3
				};

				NewNode->count = -1;

				continue;
			}

			if(
				node->count >= QUADTREE_SPLIT_THRESHOLD &&
				info.Extent.w >= QT->MinSize &&
				info.Extent.h >= QT->MinSize
				)
			{
				uint32_t ChildIdxs[4];

				for(int i = 0; i < 4; ++i)
				{
					uint32_t ChildIdx;

					if(FreeNode)
					{
						ChildIdx = FreeNode;
						FreeNode = Nodes[ChildIdx].next;
					}
					else
					{
						if(NodesUsed >= NodesSize)
						{
							uint32_t new_size = (NodesUsed << 1) | 1;

							Nodes = QT_REMALLOC(sizeof(*Nodes) * NodesSize,
								Nodes, sizeof(*Nodes) * new_size);
							assert_not_null(Nodes);

							NodesSize = new_size;

							node = Nodes + info.NodeIdx;


							if(new_size > NewNodesSize)
							{
								NewNodes = QT_REMALLOC(sizeof(*Nodes) * NewNodesSize,
									NewNodes, sizeof(*Nodes) * new_size);
								assert_not_null(NewNodes);

								NewNodesSize = new_size;

								NewNode = NewNodes + NewNodeIdx;
							}
						}

						ChildIdx = NodesUsed++;
					}

					ChildIdxs[i] = ChildIdx;
				}

				QuadtreeNode* Children[4];
				uint32_t head = node->head;
				uint32_t PositionFlags = node->PositionFlags;

				for(int i = 0; i < 4; ++i)
				{
					uint32_t ChildIdx = ChildIdxs[i];
					QuadtreeNode* Child = Nodes + ChildIdx;
					Children[i] = Child;

					node->Heads[i] = ChildIdx;

					Child->head = 0;
					Child->count = 0;

					static const uint32_t PositionFlagsMask[4] =
					{
						0b0011, /* TR */
						0b1001, /* TL */
						0b0110, /* BR */
						0b1100  /* BL */
					};

					Child->PositionFlags = PositionFlags & PositionFlagsMask[i];
				}

				uint32_t NodeEntityIdx = head;
				while(NodeEntityIdx)
				{
					QuadtreeNodeEntity* NodeEntity = NodeEntities + NodeEntityIdx;

					uint32_t EntityIdx = NodeEntity->Entity;
					QuadtreeEntity* Entity = Entities + EntityIdx;

					QuadtreeRectExtent EntityExtent = QuadtreeGetEntityRectExtent(Entity);

					uint32_t TargetNodeIdxs[4];
					uint32_t* CurrentTargetNodeIdx = TargetNodeIdxs;

					if(EntityExtent.min_x <= info.Extent.x)
					{
						if(EntityExtent.min_y <= info.Extent.y)
						{
							*(CurrentTargetNodeIdx++) = 0;
						}
						if(EntityExtent.max_y >= info.Extent.y)
						{
							*(CurrentTargetNodeIdx++) = 1;
						}
					}
					if(EntityExtent.max_x >= info.Extent.x)
					{
						if(EntityExtent.min_y <= info.Extent.y)
						{
							*(CurrentTargetNodeIdx++) = 2;
						}
						if(EntityExtent.max_y >= info.Extent.y)
						{
							*(CurrentTargetNodeIdx++) = 3;
						}
					}

					for(uint32_t* TargetNodeIdx = TargetNodeIdxs; TargetNodeIdx != CurrentTargetNodeIdx; ++TargetNodeIdx)
					{
						QuadtreeNode* TargetNode = Children[*TargetNodeIdx];

						uint32_t NewNodeEntityIdx;
						QuadtreeNodeEntity* NewNodeEntity;

						if(FreeNodeEntity)
						{
							NewNodeEntityIdx = FreeNodeEntity;
							NewNodeEntity = NodeEntities + NewNodeEntityIdx;
							FreeNodeEntity = NewNodeEntity->next;
						}
						else
						{
							if(NodeEntitiesUsed >= NodeEntitiesSize)
							{
								uint32_t new_size = (NodeEntitiesUsed << 1) | 1;

								NodeEntities = QT_REMALLOC(sizeof(*NodeEntities) * NodeEntitiesSize,
									NodeEntities, sizeof(*NodeEntities) * new_size);
								assert_not_null(NodeEntities);

								NodeEntitiesSize = new_size;

								NodeEntity = NodeEntities + NodeEntityIdx;


								if(new_size > NewNodeEntitiesSize)
								{
									NewNodeEntities = QT_REMALLOC(sizeof(*NewNodeEntities) * NewNodeEntitiesSize,
										NewNodeEntities, sizeof(*NewNodeEntities) * new_size);
									assert_not_null(NewNodeEntities);

									NewNodeEntitiesSize = new_size;
								}
							}

							NewNodeEntityIdx = NodeEntitiesUsed++;
							NewNodeEntity = NodeEntities + NewNodeEntityIdx;
						}

						NewNodeEntity->next = TargetNode->head;
						NewNodeEntity->Entity = EntityIdx;
						TargetNode->head = NewNodeEntityIdx;

						++TargetNode->count;
					}

					uint32_t NextNodeEntityIdx = NodeEntity->next;

					NodeEntity->next = FreeNodeEntity;
					FreeNodeEntity = NodeEntityIdx;

					NodeEntityIdx = NextNodeEntityIdx;
				}

				node->count = -1;

				goto goto_parent;
			}

			goto_leaf:

			NewNode->PositionFlags = node->PositionFlags;

			if(!node->head)
			{
				NewNode->head = 0;
				NewNode->count = 0;

				continue;
			}

			uint32_t NodeEntityIdx = node->head;

			NewNode->head = NewNodeEntitiesUsed;
			NewNode->count = node->count;

			while(1)
			{
				QuadtreeNodeEntity* NodeEntity = NodeEntities + NodeEntityIdx;
				QuadtreeNodeEntity* NewNodeEntity = NewNodeEntities + NewNodeEntitiesUsed;
				++NewNodeEntitiesUsed;

				uint32_t EntityIdx = NodeEntity->Entity;
				if(!EntityMap[EntityIdx])
				{
					uint32_t NewEntityIdx = NewEntitiesUsed++;
					EntityMap[EntityIdx] = NewEntityIdx;
					NewEntities[NewEntityIdx] = Entities[EntityIdx];
				}

				NewNodeEntity->Entity = EntityMap[EntityIdx];

				if(NodeEntity->next)
				{
					NodeEntityIdx = NodeEntity->next;
					NewNodeEntity->next = NewNodeEntitiesUsed;
				}
				else
				{
					NewNodeEntity->next = 0;
					break;
				}
			}
		}
		while(NodeInfo != NodeInfos);

		QT_FREE(sizeof(*Nodes) * NodesSize, Nodes);
		QT->Nodes = NewNodes;
		QT->NodesUsed = NewNodesUsed;
		QT->NodesSize = NewNodesSize;

		QT_FREE(sizeof(*NodeEntities) * NodeEntitiesSize, NodeEntities);
		QT->NodeEntities = NewNodeEntities;
		QT->NodeEntitiesUsed = NewNodeEntitiesUsed;
		QT->NodeEntitiesSize = NewNodeEntitiesSize;

		QT_FREE(sizeof(*Entities) * EntitiesSize, Entities);
		QT->Entities = NewEntities;
		QT->EntitiesUsed = NewEntitiesUsed;
		QT->EntitiesSize = NewEntitiesSize;

		QT_FREE(sizeof(*EntityMap) * EntitiesSize, EntityMap);
	}
}


void
QuadtreeUpdate(
	Quadtree* QT,
	QuadtreeUpdateT Callback
	)
{
	QT->UpdateTick ^= 1;
	uint32_t UpdateTick = QT->UpdateTick;

	QuadtreeNode* Nodes = QT->Nodes;
	QuadtreeNodeEntity* NodeEntities = QT->NodeEntities;
	QuadtreeEntity* Entities = QT->Entities;
	QuadtreeReinsertion* Reinsertions = QT->Reinsertions;
	QuadtreeNodeRemoval* NodeRemovals = QT->NodeRemovals;

	uint32_t ReinsertionsUsed = QT->ReinsertionsUsed;
	uint32_t ReinsertionsSize = QT->ReinsertionsSize;

	uint32_t NodeRemovalsUsed = QT->NodeRemovalsUsed;
	uint32_t NodeRemovalsSize = QT->NodeRemovalsSize;

	QuadtreeNodeInfo NodeInfos[QUADTREE_DFS_LENGTH];
	QuadtreeNodeInfo* NodeInfo = NodeInfos;

	*(NodeInfo++) =
	(QuadtreeNodeInfo)
	{
		.NodeIdx = 0,
		.Extent = QT->half_extent_t
	};

	do
	{
		QuadtreeNodeInfo info = *(--NodeInfo);
		QuadtreeNode* node = Nodes + info.NodeIdx;

		if(node->count == -1)
		{
			QuadtreeDescend(((QuadtreeRectExtent){ info.Extent.x, info.Extent.x, info.Extent.y, info.Extent.y }));
			continue;
		}

		QuadtreeRectExtent NodeExtent = QuadtreeHalfToRectExtent(info.Extent);

		uint32_t PrevIdx = 0;
		uint32_t Idx = node->head;

		while(Idx)
		{
			QuadtreeNodeEntity* NodeEntity = NodeEntities + Idx;

			uint32_t EntityIdx = NodeEntity->Entity;
			QuadtreeEntity* Entity = Entities + EntityIdx;

			QuadtreeRectExtent Extent;

			if(Entity->UpdateTick != UpdateTick)
			{
				Entity->UpdateTick = UpdateTick;
				QuadtreeStatus status = Callback(QT, EntityIdx, &Entity->data);
				Extent = QuadtreeGetEntityRectExtent(Entity);

				if(status == QUADTREE_STATUS_CHANGED && !QuadtreeIsInside(Extent, NodeExtent))
				{
					uint32_t ReinsertionIdx;
					QuadtreeReinsertion* Reinsertion;

					if(ReinsertionsUsed >= ReinsertionsSize)
					{
						uint32_t new_size = (ReinsertionsUsed << 1) | 1;

						Reinsertions = QT_REMALLOC(sizeof(*Reinsertions) * ReinsertionsSize,
							Reinsertions, sizeof(*Reinsertions) * new_size);
						assert_not_null(Reinsertions);

						ReinsertionsSize = new_size;
					}

					ReinsertionIdx = ReinsertionsUsed++;
					Reinsertion = Reinsertions + ReinsertionIdx;

					Reinsertion->EntityIdx = EntityIdx;
				}
			}
			else
			{
				Extent = QuadtreeGetEntityRectExtent(Entity);
			}

			if(
				(Extent.max_x < NodeExtent.min_x && !(node->PositionFlags & 0b0001)) ||
				(Extent.max_y < NodeExtent.min_y && !(node->PositionFlags & 0b0010)) ||
				(NodeExtent.max_x < Extent.min_x && !(node->PositionFlags & 0b0100)) ||
				(NodeExtent.max_y < Extent.min_y && !(node->PositionFlags & 0b1000))
				)
			{
				uint32_t NodeRemovalIdx;
				QuadtreeNodeRemoval* NodeRemoval;

				if(NodeRemovalsUsed >= NodeRemovalsSize)
				{
					uint32_t new_size = (NodeRemovalsUsed << 1) | 1;

					NodeRemovals = QT_REMALLOC(sizeof(*NodeRemovals) * NodeRemovalsSize,
						NodeRemovals, sizeof(*NodeRemovals) * new_size);
					assert_not_null(NodeRemovals);

					NodeRemovalsSize = new_size;
				}

				NodeRemovalIdx = NodeRemovalsUsed++;
				NodeRemoval = NodeRemovals + NodeRemovalIdx;

				NodeRemoval->NodeIdx = info.NodeIdx;
				NodeRemoval->NodeEntityIdx = Idx;
				NodeRemoval->PrevNodeEntityIdx = PrevIdx;
			}

			PrevIdx = Idx;
			Idx = NodeEntity->next;
		}
	}
	while(NodeInfo != NodeInfos);

	QT->Reinsertions = Reinsertions;
	QT->ReinsertionsUsed = ReinsertionsUsed;
	QT->ReinsertionsSize = ReinsertionsSize;

	QT->NodeRemovals = NodeRemovals;
	QT->NodeRemovalsUsed = NodeRemovalsUsed;
	QT->NodeRemovalsSize = NodeRemovalsSize;
}


void
QuadtreeQuery(
	Quadtree* QT,
	QuadtreeRectExtent Extent,
	QuadtreeQueryT Callback
	)
{
	++QT->QueryTick;
	uint32_t QueryTick = QT->QueryTick;

	QuadtreeNode* Nodes = QT->Nodes;
	QuadtreeNodeEntity* NodeEntities = QT->NodeEntities;
	QuadtreeEntity* Entities = QT->Entities;

	QuadtreeNodeInfo NodeInfos[QUADTREE_DFS_LENGTH];
	QuadtreeNodeInfo* NodeInfo = NodeInfos;

	*(NodeInfo++) =
	(QuadtreeNodeInfo)
	{
		.NodeIdx = 0,
		.Extent = QT->half_extent_t
	};

	do
	{
		QuadtreeNodeInfo info = *(--NodeInfo);
		QuadtreeNode* node = Nodes + info.NodeIdx;

		if(node->count == -1)
		{
			QuadtreeDescend(Extent);
			continue;
		}

		uint32_t Idx = node->head;
		while(Idx)
		{
			QuadtreeNodeEntity* NodeEntity = NodeEntities + Idx;
			uint32_t EntityIdx = NodeEntity->Entity;
			QuadtreeEntity* Entity = Entities + EntityIdx;

			if(Entity->QueryTick != QueryTick)
			{
				Entity->QueryTick = QueryTick;

				if(QuadtreeIntersects(QuadtreeGetEntityRectExtent(Entity), Extent))
				{
					Callback(QT, EntityIdx, &Entity->data);
				}
			}

			Idx = NodeEntity->next;
		}
	}
	while(NodeInfo != NodeInfos);
}


void
QuadtreeQueryNodes(
	Quadtree* QT,
	QuadtreeRectExtent Extent,
	QuadtreeNodeQueryT Callback
	)
{
	QuadtreeNode* Nodes = QT->Nodes;

	QuadtreeNodeInfo NodeInfos[QUADTREE_DFS_LENGTH];
	QuadtreeNodeInfo* NodeInfo = NodeInfos;

	*(NodeInfo++) =
	(QuadtreeNodeInfo)
	{
		.NodeIdx = 0,
		.Extent = QT->half_extent_t
	};

	do
	{
		QuadtreeNodeInfo info = *(--NodeInfo);
		QuadtreeNode* node = Nodes + info.NodeIdx;

		if(node->count == -1)
		{
			QuadtreeDescend(Extent);
			continue;
		}

		Callback(QT, &info);
	}
	while(NodeInfo != NodeInfos);
}


void
QuadtreeCollide(
	Quadtree* QT,
	QuadtreeCollideT Callback
	)
{
	QuadtreeNormalize(QT);

	if(QT->EntitiesUsed <= 1)
	{
		return;
	}

#if QUADTREE_DEDUPE_COLLISIONS == 1
	uint32_t HashTableSize = QT->EntitiesUsed * QUADTREE_HASH_TABLE_FACTOR;
	uint32_t* hash_table_t = QT_CALLOC(sizeof(*hash_table_t) * HashTableSize);
	assert_not_null(hash_table_t);

	QuadtreeHTEntry* HTEntries = QT->HTEntries;

	uint32_t HTEntriesUsed = 1;
	uint32_t HTEntriesSize = QT->HTEntriesSize;
#endif

	QuadtreeNodeEntity* NodeEntities = QT->NodeEntities;
	QuadtreeEntity* Entities = QT->Entities;

	QuadtreeNodeEntity* NodeEntity = NodeEntities;
	QuadtreeNodeEntity* NodeEntitiesEnd = NodeEntities + QT->NodeEntitiesUsed - 1;

	do
	{
		++NodeEntity;

		if(!NodeEntity->next)
		{
			continue;
		}

		uint32_t EntityIdx = NodeEntity->Entity;
		QuadtreeEntity* Entity = Entities + EntityIdx;
		QuadtreeRectExtent EntityExtent = QuadtreeGetEntityRectExtent(Entity);

		QuadtreeNodeEntity* OtherNodeEntity = NodeEntity;
		while(1)
		{
			++OtherNodeEntity;

			uint32_t OtherEntityIdx = OtherNodeEntity->Entity;
			QuadtreeEntity* OtherEntity = Entities + OtherEntityIdx;

			if(!QuadtreeIntersects(
				EntityExtent,
				QuadtreeGetEntityRectExtent(OtherEntity)
				))
			{
				goto goto_skip;
			}

#if QUADTREE_DEDUPE_COLLISIONS == 1
			uint32_t idx_a = EntityIdx;
			uint32_t idx_b = OtherEntityIdx;

			if(idx_a > idx_b)
			{
				uint32_t Temp = idx_a;
				idx_a = idx_b;
				idx_b = Temp;
			}

			uint32_t hash = idx_a * 48611 + idx_b * 50261;
			hash %= HashTableSize;

			uint32_t idx = hash_table_t[hash];
			QuadtreeHTEntry* entry;

			while(idx)
			{
				entry = HTEntries + idx;

				if(entry->Idx[0] == idx_a && entry->Idx[1] == idx_b)
				{
					goto goto_skip;
				}

				idx = entry->next;
			}

			if(HTEntriesUsed >= HTEntriesSize)
			{
				uint32_t new_size = (HTEntriesUsed << 1) | 1;

				HTEntries = QT_REMALLOC(sizeof(*HTEntries) * HTEntriesSize,
					HTEntries, sizeof(*HTEntries) * new_size);
				assert_not_null(HTEntries);

				HTEntriesSize = new_size;
			}

			uint32_t EntryIdx = HTEntriesUsed++;
			entry = HTEntries + EntryIdx;

			entry->Idx[0] = idx_a;
			entry->Idx[1] = idx_b;
			entry->next = idx;
			hash_table_t[hash] = EntryIdx;
#endif

			Callback(QT, &Entity->data, &OtherEntity->data);

			goto_skip:;

			if(!OtherNodeEntity->next)
			{
				break;
			}
		}
	}
	while(NodeEntity != NodeEntitiesEnd);

#if QUADTREE_DEDUPE_COLLISIONS == 1
	QT->HTEntries = HTEntries;
	QT->HTEntriesSize = HTEntriesSize;

	QT_FREE(sizeof(*hash_table_t) * HashTableSize, hash_table_t);
#endif
}


#undef QuadtreeDescend
