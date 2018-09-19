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
	/// Parse the config file at startup
	BatcherCodec(const CodecConstructorParameters &params) : AbstractCodec(params) {
		if (params.getConfig().size() != 0) {
			throw std::runtime_error("Unexpected configuration");
		}
	}

	virtual void sendBatchTowardsTransport(Message *start, Message *end) {
		if (start == end) return;
		list_t list;
		for (Message *it = start; it != end; ++it) {
			list.push_back(std::move(it->getPayload()));
		}
		Message msg { data_t(std::move(list)), std::move(start->getMetadata()) };
		transportSide->sendBatchTowardsTransport(&msg, &msg + 1);
	}

	virtual void sendBatchTowardsHost(Message *start, Message *end) {
		auto latest = start;
		for (auto it = start; it != end; ++it) {
			auto &payload = it->getPayload();
			if (SAG_DATA_LIST == payload.type_tag()) {
				auto &l = get<list_t>(payload);
				if (latest != it) hostSide->sendBatchTowardsHost(latest, it);
				std::unique_ptr<Message[]> ms(new Message[l.size()]);
				size_t i = 0;
				for (auto jt = l.begin(); jt != l.end(); ++jt) {
					ms[i++] = Message{std::move(*jt), it->getMetadataMap().copy()};
				}

				hostSide->sendBatchTowardsHost(ms.get(), ms.get()+l.size());
				latest = it+1;
			}
		}
		if (latest != end) hostSide->sendBatchTowardsHost(latest, end);
	}

};
/// Export this codec
SAG_DECLARE_CONNECTIVITY_CODEC_CLASS(BatcherCodec);

}}
