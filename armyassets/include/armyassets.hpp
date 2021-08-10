#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>

#include <checkformat.hpp>
#include <armydata.hpp>

using namespace eosio;
using namespace std;
using namespace armydata;


static constexpr double MAX_MARKET_FEE = 0.15;


CONTRACT armyassets : public contract {
public:
    using contract::contract;

    ACTION init();

    ACTION admincoledit(vector <FORMAT> collection_format_extension);

    ACTION createcol(
        name author,
        name collection_name,
        bool allow_notify,
        vector <name> authorized_accounts,
        vector <name> notify_accounts,
        double market_fee,
        ATTRIBUTE_MAP data
    );

    ACTION createschema(
        name authorized_creator,
        name collection_name,
        name schema_name,
        vector <FORMAT> schema_format
    );

    ACTION createtempl(
        name authorized_creator,
        name collection_name,
        name schema_name,
        bool transferable,
        bool burnable,
        uint32_t max_supply,
        ATTRIBUTE_MAP immutable_data
    );

    ACTION mintasset(
        name authorized_minter,
        name collection_name,
        name schema_name,
        int32_t template_id,
        name new_asset_owner,
        ATTRIBUTE_MAP immutable_data,
        ATTRIBUTE_MAP mutable_data,
        vector <asset> tokens_to_back
    );

    ACTION lognewtempl(
        int32_t template_id,
        name authorized_creator,
        name collection_name,
        name schema_name,
        bool transferable,
        bool burnable,
        uint32_t max_supply,
        ATTRIBUTE_MAP immutable_data
    );

    ACTION logmint(
        uint64_t asset_id,
        name authorized_minter,
        name collection_name,
        name schema_name,
        int32_t template_id,
        name new_asset_owner,
        ATTRIBUTE_MAP immutable_data,
        ATTRIBUTE_MAP mutable_data,
        vector <asset> backed_tokens,
        ATTRIBUTE_MAP immutable_template_data
    );

private:

    TABLE collections_s {
        name             collection_name;
        name             author;
        bool             allow_notify;
        vector <name>    authorized_accounts;
        vector <name>    notify_accounts;
        double           market_fee;
        vector <uint8_t> serialized_data;

        uint64_t primary_key() const { return collection_name.value; };
    };

    typedef multi_index <name("collections"), collections_s> collections_t;


    //Scope: collection_name
    TABLE schemas_s {
        name            schema_name;
        vector <FORMAT> format;

        uint64_t primary_key() const { return schema_name.value; }
    };

    typedef multi_index <name("schemas"), schemas_s> schemas_t;


    //Scope: collection_name
    TABLE templates_s {
        int32_t          template_id;
        name             schema_name;
        bool             transferable;
        bool             burnable;
        uint32_t         max_supply;
        uint32_t         issued_supply;
        vector <uint8_t> immutable_serialized_data;

        uint64_t primary_key() const { return (uint64_t) template_id; }
    };

    typedef multi_index <name("templates"), templates_s> templates_t;


    //Scope: owner
    TABLE assets_s {
        uint64_t         asset_id;
        name             collection_name;
        name             schema_name;
        int32_t          template_id;
        name             ram_payer;
        vector <asset>   backed_tokens;
        vector <uint8_t> immutable_serialized_data;
        vector <uint8_t> mutable_serialized_data;

        uint64_t primary_key() const { return asset_id; };
    };

    typedef multi_index <name("assets"), assets_s> assets_t;


    TABLE config_s {
        uint64_t                 asset_counter     = 1099511627776; //2^40
        int32_t                  template_counter  = 1;
        uint64_t                 offer_counter     = 1;
        vector <FORMAT>          collection_format = {};
        vector <extended_symbol> supported_tokens  = {};
    };
    typedef singleton <name("config"), config_s>               config_t;
    // https://github.com/EOSIO/eosio.cdt/issues/280
    typedef multi_index <name("config"), config_s>             config_t_for_abi;

    TABLE tokenconfigs_s {
        name        standard = name("armyassets");
        std::string version  = string("1.3.1");
    };
    typedef singleton <name("tokenconfigs"), tokenconfigs_s>   tokenconfigs_t;
    // https://github.com/EOSIO/eosio.cdt/issues/280
    typedef multi_index <name("tokenconfigs"), tokenconfigs_s> tokenconfigs_t_for_abi;


    collections_t  collections  = collections_t(get_self(), get_self().value);
    config_t       config       = config_t(get_self(), get_self().value);
    tokenconfigs_t tokenconfigs = tokenconfigs_t(get_self(), get_self().value);


    void notify_collection_accounts(
        name collection_name
    );

    void check_has_collection_auth(
        name account_to_check,
        name collection_name,
        string error_message
    );

    void check_name_length(ATTRIBUTE_MAP data);

    assets_t get_assets(name acc);

    schemas_t get_schemas(name collection_name);

    templates_t get_templates(name collection_name);
};
