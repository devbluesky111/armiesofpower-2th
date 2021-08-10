#include <armyassets.hpp>

/**
*  Initializes the config table. Only needs to be called once when first deploying the contract
*  @required_auth The contract itself
*/
ACTION armyassets::init() {
    require_auth(get_self());
    config.get_or_create(get_self(), config_s{});
    tokenconfigs.get_or_create(get_self(), tokenconfigs_s{});
}

/**
*  Adds one or more lines to the format that is used for collection data serialization
*  @required_auth The contract itself
*/
ACTION armyassets::admincoledit(vector <armydata::FORMAT> collection_format_extension) {
    require_auth(get_self());

    check(collection_format_extension.size() != 0, "Need to add at least one new line");

    config_s current_config = config.get();
    current_config.collection_format.insert(
        current_config.collection_format.end(),
        collection_format_extension.begin(),
        collection_format_extension.end()
    );
    check_format(current_config.collection_format);

    config.set(current_config, get_self());
}


/**
*  Creates a new collection
*/
ACTION armyassets::createcol(
    name author,
    name collection_name,
    bool allow_notify,
    vector <name> authorized_accounts,
    vector <name> notify_accounts,
    double market_fee,
    ATTRIBUTE_MAP data
) {
    require_auth(author);

    name collection_name_suffix = collection_name.suffix();

    if (is_account(collection_name)) {
        check(has_auth(collection_name),
            "When the collection has the name of an existing account, its authorization is required");
    } else {
        if (collection_name_suffix != collection_name) {
            check(has_auth(collection_name_suffix),
                "When the collection name has a suffix, the suffix authorization is required");
        } else {
            check(collection_name.length() == 12,
                "Without special authorization, collection names must be 12 characters long");
        }
    }

    check(collections.find(collection_name.value) == collections.end(),
        "A collection with this name already exists");

    check(allow_notify || notify_accounts.size() == 0, "Can't add notify_accounts if allow_notify is false");

    for (auto itr = authorized_accounts.begin(); itr != authorized_accounts.end(); itr++) {
        check(is_account(*itr), string("At least one account does not exist - " + itr->to_string()).c_str());
        check(std::find(authorized_accounts.begin(), authorized_accounts.end(), *itr) == itr,
            "You can't have duplicates in the authorized_accounts");
    }
    for (auto itr = notify_accounts.begin(); itr != notify_accounts.end(); itr++) {
        check(is_account(*itr), string("At least one account does not exist - " + itr->to_string()).c_str());
        check(std::find(notify_accounts.begin(), notify_accounts.end(), *itr) == itr,
            "You can't have duplicates in the notify_accounts");
    }

    check(0 <= market_fee && market_fee <= MAX_MARKET_FEE,
        "The market_fee must be between 0 and " + to_string(MAX_MARKET_FEE));

    check_name_length(data);

    config_s current_config = config.get();

    collections.emplace(author, [&](auto &_collection) {
        _collection.collection_name = collection_name;
        _collection.author = author;
        _collection.allow_notify = allow_notify;
        _collection.authorized_accounts = authorized_accounts;
        _collection.notify_accounts = notify_accounts;
        _collection.market_fee = market_fee;
        _collection.serialized_data = serialize(data, current_config.collection_format);
    });
}

/**
*  Creates a new schema
*  schemas can only be extended in the future, but never changed retroactively.
*  This guarantees a correct deserialization for existing templates and assets.
*  @required_auth authorized_creator, who is within the authorized_accounts list of the collection
*/
ACTION armyassets::createschema(
    name authorized_creator,
    name collection_name,
    name schema_name,
    vector <FORMAT> schema_format
) {
    require_auth(authorized_creator);
    
    check(schema_name.length() == 12,
        "Schema names must be 12 characters long");

    auto collection_itr = collections.require_find(collection_name.value,
        "No collection with this name exists");

    check_has_collection_auth(
        authorized_creator,
        collection_name,
        "The creator is not authorized within the collection"
    );

    schemas_t collection_schemas = get_schemas(collection_name);

    check(collection_schemas.find(schema_name.value) == collection_schemas.end(),
        "A schema with this name already exists for this collection");

    check_format(schema_format);

    collection_schemas.emplace(authorized_creator, [&](auto &_schema) {
        _schema.schema_name = schema_name;
        _schema.format = schema_format;
    });
}

/**
*  Creates a new template
*  @required_auth authorized_creator, who is within the authorized_accounts list of the collection
*/
ACTION armyassets::createtempl(
    name authorized_creator,
    name collection_name,
    name schema_name,
    bool transferable,
    bool burnable,
    uint32_t max_supply,
    ATTRIBUTE_MAP immutable_data
) {
    require_auth(authorized_creator);

    auto collection_itr = collections.require_find(collection_name.value,
        "No collection with this name exists");

    check_has_collection_auth(
        authorized_creator,
        collection_name,
        "The creator is not authorized within the collection"
    );

    schemas_t collection_schemas = get_schemas(collection_name);
    auto schema_itr = collection_schemas.require_find(schema_name.value,
        "No schema with this name exists");

    config_s current_config = config.get();
    int32_t template_id = current_config.template_counter++;
    config.set(current_config, get_self());

    templates_t collection_templates = get_templates(collection_name);

    collection_templates.emplace(authorized_creator, [&](auto &_template) {
        _template.template_id = template_id;
        _template.schema_name = schema_name;
        _template.transferable = transferable;
        _template.burnable = burnable;
        _template.max_supply = max_supply;
        _template.issued_supply = 0;
        _template.immutable_serialized_data = serialize(immutable_data, schema_itr->format);
    });

    action(
        permission_level{get_self(), name("active")},
        get_self(),
        name("lognewtempl"),
        make_tuple(
            template_id,
            authorized_creator,
            collection_name,
            schema_name,
            transferable,
            burnable,
            max_supply,
            immutable_data
        )
    ).send();
}

/**
*  Creates a new asset
*  Doesn't work if the template has a specified max_supply that has already been reached
*  @required_auth authorized_minter, who is within the authorized_accounts list of the collection
                  specified in the related template
*/
ACTION armyassets::mintasset(
    name authorized_minter,
    name collection_name,
    name schema_name,
    int32_t template_id,
    name new_asset_owner,
    ATTRIBUTE_MAP immutable_data,
    ATTRIBUTE_MAP mutable_data,
    vector <asset> tokens_to_back
) {
    require_auth(authorized_minter);

    auto collection_itr = collections.find(collection_name.value);

    check_has_collection_auth(
        authorized_minter,
        collection_name,
        "The minter is not authorized within the collection"
    );

    schemas_t collection_schemas = get_schemas(collection_name);
    auto schema_itr = collection_schemas.require_find(schema_name.value,
        "No schema with this name exists");

    //Needed for the log action
    ATTRIBUTE_MAP deserialized_template_data;
    if (template_id >= 0) {
        templates_t collection_templates = get_templates(collection_name);

        auto template_itr = collection_templates.require_find(template_id,
            "No template with this id exists");

        check(template_itr->schema_name == schema_name,
            "The template belongs to another schema");

        if (template_itr->max_supply > 0) {
            check(template_itr->issued_supply < template_itr->max_supply,
                "The template's maxsupply has already been reached");
        }
        collection_templates.modify(template_itr, same_payer, [&](auto &_template) {
            _template.issued_supply += 1;
        });

        deserialized_template_data = deserialize(
            template_itr->immutable_serialized_data,
            schema_itr->format
        );
    } else {
        check(template_id == -1, "The template id must either be an existing template or -1");

        deserialized_template_data = {};
    }

    check(is_account(new_asset_owner), "The new_asset_owner account does not exist");

    check_name_length(immutable_data);
    check_name_length(mutable_data);

    config_s current_config = config.get();
    uint64_t asset_id = current_config.asset_counter++;
    config.set(current_config, get_self());

    assets_t new_owner_assets = get_assets(new_asset_owner);
    new_owner_assets.emplace(authorized_minter, [&](auto &_asset) {
        _asset.asset_id = asset_id;
        _asset.collection_name = collection_name;
        _asset.schema_name = schema_name;
        _asset.template_id = template_id;
        _asset.ram_payer = authorized_minter;
        _asset.backed_tokens = {};
        _asset.immutable_serialized_data = serialize(immutable_data, schema_itr->format);
        _asset.mutable_serialized_data = serialize(mutable_data, schema_itr->format);
    });


    action(
        permission_level{get_self(), name("active")},
        get_self(),
        name("logmint"),
        make_tuple(
            asset_id,
            authorized_minter,
            collection_name,
            schema_name,
            template_id,
            new_asset_owner,
            immutable_data,
            mutable_data,
            tokens_to_back,
            deserialized_template_data
        )
    ).send();
}

ACTION armyassets::lognewtempl(
    int32_t template_id,
    name authorized_creator,
    name collection_name,
    name schema_name,
    bool transferable,
    bool burnable,
    uint32_t max_supply,
    ATTRIBUTE_MAP immutable_data
) {
    require_auth(get_self());

    notify_collection_accounts(collection_name);
}


ACTION armyassets::logmint(
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
) {
    require_auth(get_self());

    require_recipient(new_asset_owner);

    notify_collection_accounts(collection_name);
}

/**
* Notifies all of a collection's notify accounts using require_recipient
*/
void armyassets::notify_collection_accounts(
    name collection_name
) {
    auto collection_itr = collections.require_find(collection_name.value,
        "No collection with this name exists");

    for (const name &notify_account : collection_itr->notify_accounts) {
        require_recipient(notify_account);
    }
}


/**
* Checks if the account_to_check is in the authorized_accounts vector of the specified collection
*/
void armyassets::check_has_collection_auth(
    name account_to_check,
    name collection_name,
    string error_message
) {
    auto collection_itr = collections.require_find(collection_name.value,
        "No collection with this name exists");

    check(std::find(
        collection_itr->authorized_accounts.begin(),
        collection_itr->authorized_accounts.end(),
        account_to_check
        ) != collection_itr->authorized_accounts.end(),
        error_message);
}


/**
* The "name" attribute is limited to 64 characters max for both assets and collections
* This function checks that, if there exists an ATTRIBUTE with name: "name", the value of it
* must be of length <= 64
*/
void armyassets::check_name_length(
    ATTRIBUTE_MAP data
) {
    auto data_itr = data.find("name");
    if (data_itr != data.end()) {
        if (std::holds_alternative <string>(data_itr->second)) {
            check(std::get <string>(data_itr->second).length() <= 64,
                "Names (attribute with name: \"name\") can only be 64 characters max");
        }
    }
}


armyassets::assets_t armyassets::get_assets(name acc) {
    return assets_t(get_self(), acc.value);
}


armyassets::schemas_t armyassets::get_schemas(name collection_name) {
    return schemas_t(get_self(), collection_name.value);
}


armyassets::templates_t armyassets::get_templates(name collection_name) {
    return templates_t(get_self(), collection_name.value);
}
