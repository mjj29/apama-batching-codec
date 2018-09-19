#include<sag_connectivity_plugins.hpp>
#include<string>

using namespace com::softwareag::connectivity;

namespace com {
namespace apamax {

class EchoTransport: public AbstractSimpleTransport
{
public:
	explicit EchoTransport(const TransportConstructorParameters &param)
		: AbstractSimpleTransport(param)
	{}
	virtual void deliverMessageTowardsTransport(Message &m) override
	{
		hostSide->sendBatchTowardsHost(&m, &m+1);
	}
};

SAG_DECLARE_CONNECTIVITY_TRANSPORT_CLASS(EchoTransport)

}} // com::apamax

