/**
 * Title:        BatcherCodec.cpp
 * Description:  Batcher Codec
 * $Copyright (c) 2016, 2017 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$
 * Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG
 * $Revision$ $Date$
 */

#include <sag_connectivity_plugins.hpp>
#include <vector>

using namespace com::softwareag::connectivity;

namespace com {
namespace apamax {

/**
 * This codec maps a batch of incoming events into a single list.
 * The metadata is assumed to be the same.
 *
 * @author CREE
 */
class BatcherCodec: public AbstractCodec
{
public:
	enum MetadataMode { METADATA_FIRST, METADATA_MEMBER, METADATA_REQUESTID_LIST, METADATA_SPLIT_BATCH };
	/// Parse the config file at startup
	BatcherCodec(const CodecConstructorParameters &params)
		: AbstractCodec(params),
		metadataMode(METADATA_FIRST),
		requestIdData("requestId"),
		payloadData("payload"),
		metadataData("metadata")
	{
		MapExtractor configEx(config, "config");
		std::string mode = configEx.getStringDisallowEmpty("metadataMode", "first");
		if (mode == "first") {
			metadataMode = METADATA_FIRST;
		} else if (mode == "member") {
			metadataMode = METADATA_MEMBER;
		} else if (mode == "requestIdList") {
			metadataMode = METADATA_REQUESTID_LIST;
		} else if (mode == "splitBatch") {
			metadataMode = METADATA_SPLIT_BATCH;
		} else {
			throw std::runtime_error("Unknown metadataMode. Should be first, member, requestIdList or splitBatch");
		}
		configEx.checkNoItemsRemaining();
	}

	virtual void sendBatchTowardsTransport(Message *start, Message *end) {
		if (start == end) return;
		list_t list;
		list_t requestIds;
		auto *latest = start;
		for (Message *it = start; it != end; ++it) {
			switch (metadataMode) {
				case METADATA_REQUESTID_LIST: {
					requestIds.push_back(std::move(it->getMetadataMap()[requestIdData]));
					// fallthrough
				}
				case METADATA_SPLIT_BATCH:
				case METADATA_FIRST: {
					if (metadataMode == METADATA_SPLIT_BATCH && list.size() > 0 && it->getMetadataMap() != latest->getMetadataMap()) {
						Message msg { data_t(std::move(list)), std::move(latest->getMetadataMap()) };
						transportSide->sendBatchTowardsTransport(&msg, &msg + 1);
						latest = it;
					}
					list.push_back(std::move(it->getPayload()));
					break;
				}
				case METADATA_MEMBER: {
					map_t obj;
					obj.insert(std::make_pair(payloadData.copy(), std::move(it->getPayload())));
					obj.insert(std::make_pair(metadataData.copy(), std::move(it->getMetadataMap())));
					list.push_back(std::move(obj));
					break;
				}
				default: assert(false);
			}
		}
		switch (metadataMode) {
			case METADATA_REQUESTID_LIST: {
				start->getMetadataMap()[requestIdData] = std::move(requestIds);
				// fallthrough
			}
			default: {
				Message msg { data_t(std::move(list)), std::move(latest->getMetadataMap()) };
				transportSide->sendBatchTowardsTransport(&msg, &msg + 1);
			}
		}
	}

	virtual void sendBatchTowardsHost(Message *start, Message *end) {
		auto latest = start;
		for (auto it = start; it != end; ++it) {
			auto &payload = it->getPayload();
			if (SAG_DATA_LIST == payload.type_tag()) {
				auto &l = get<list_t>(payload);
				if (latest != it) hostSide->sendBatchTowardsHost(latest, it);
				list_t::iterator rit;
				list_t::iterator rend;
				if (metadataMode == METADATA_REQUESTID_LIST) {
					rit = get<list_t>(it->getMetadataMap()[requestIdData]).begin();
					rend = get<list_t>(it->getMetadataMap()[requestIdData]).end();
				}
				std::unique_ptr<Message[]> ms(new Message[l.size()]);
				size_t i = 0;
				for (auto jt = l.begin(); jt != l.end(); ++jt) {
					switch (metadataMode) {
						case METADATA_SPLIT_BATCH:
						case METADATA_FIRST: {
							auto meta = it->getMetadataMap().copy();
							ms[i++] = Message{std::move(*jt), std::move(meta)};
							break;
						}
						case METADATA_MEMBER: {
							auto &m = get<map_t>(*jt);
							auto &meta = get<map_t>(m[metadataData]);
							auto &payload = m[payloadData];
							ms[i++] = Message{std::move(payload), std::move(meta)};
							break;
						}
						case METADATA_REQUESTID_LIST: {
							auto meta = it->getMetadataMap().copy();
							meta[requestIdData] = std::move(*rit);
							rit++;
							ms[i++] = Message{std::move(*jt), std::move(meta)};
							break;
						}
					}
				}

				hostSide->sendBatchTowardsHost(ms.get(), ms.get()+l.size());
				latest = it+1;
			}
		}
		if (latest != end) hostSide->sendBatchTowardsHost(latest, end);
	}
	MetadataMode metadataMode;
	data_t requestIdData;
	data_t payloadData;
	data_t metadataData;

};
/// Export this codec
SAG_DECLARE_CONNECTIVITY_CODEC_CLASS(BatcherCodec);

}}
