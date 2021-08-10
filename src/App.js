import './App.css';
import { useState } from 'react';
import * as XLSX from 'xlsx';

function App() {

  const [initMessage, setInitMessage] = useState('');
  const [adminColEditMessage, setAdminColEditMessage] = useState('');
  const [createColMessage, setCreateColMessage] = useState('');
  const [createSchemaMessage, setCreateSchemaMessage] = useState('');
  const [createTemplateMessage, setCreateTemplateMessage] = useState('');
  const [columns, setColumns] = useState([]);
  const [data, setData] = useState([]);

  const { Api, JsonRpc, RpcError } = require('eosjs');
  const { JsSignatureProvider } = require('eosjs/dist/eosjs-jssig');

  const defaultPrivateKey = "5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"; // bob
  const signatureProvider = new JsSignatureProvider([defaultPrivateKey]);
  const rpc = new JsonRpc('http://127.0.0.1:8888');
  const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() });

  /**
  *  Initializes the config table. Only needs to be called once when first deploying the contract
  *  Admin management
  */
  const init = async () => {
    try {
      const result = await api.transact({
        actions: [{
          account: 'armyassets',
          name: 'init',
          authorization: [{
            actor: 'armyassets',
            permission: 'active',
          }],
          data: {},
        }]
      }, {
        blocksBehind: 3,
        expireSeconds: 1200,
      });
      setInitMessage(JSON.stringify(result));
      await new Promise(resolve => setTimeout(resolve, 1000));
      // await getCurrentMessage();
    } catch(e) {
      setInitMessage(e.message);
    }
  }

  /**
  *  Adds one or more lines to the format that is used for collection data serialization
  *  Admin management
  */
  const admincoledit = async () => {
  
    try {
      const result = await api.transact({
        actions: [{
          account: 'armyassets',
          name: 'admincoledit',
          authorization: [{
            actor: 'armyassets',
            permission: 'active',
          }],
          data: {
            collection_format_extension: [
              {
                name:"name",
                type:"string",
              },
              {
                name:"websiteUrl",
                type:"string",
              },
              {
                name:"description",
                type:"string",
              }
            ],
          },
        }]
      }, {
        blocksBehind: 3,
        expireSeconds: 1200,
      });
      setAdminColEditMessage(JSON.stringify(result));
      await new Promise(resolve => setTimeout(resolve, 1000));
      // await getCurrentMessage();
    } catch(e) {
      setAdminColEditMessage(e.message);
      if (e instanceof RpcError)
        console.log(JSON.stringify(e.json, null, 2));
    }
  }

  /**
  *  Create collection
  *  User Transaction
  */
  const createcol = async () => {
  
    try {
      const result = await api.transact({
        actions: [{
          account: 'armyassets',
          name: 'createcol',
          authorization: [{
            actor: 'armyassets',
            permission: 'active',
          }],
          data: {
            author: 'armyassets',
            collection_name: "aacollection",
            allow_notify: true,
            authorized_accounts: ['armyassets'],
            notify_accounts: ['armyassets'],
            market_fee: 0.1,
            data: [
              {
                key:"name",
                value:["string","aacollection"],
              },
              {
                key:"websiteUrl",
                value:["string","http://armiesofpower.com"],
              },
              {
                key:"description",
                value:["string","This is the test collection for armiesofpower"],
              },
            ],
          },
        }]
      }, {
        blocksBehind: 3,
        expireSeconds: 1200,
      });
      setCreateColMessage(JSON.stringify(result));
      await new Promise(resolve => setTimeout(resolve, 1000));
      // await getCurrentMessage();
    } catch(e) {
      setCreateColMessage(e.message);
      if (e instanceof RpcError)
        console.log(JSON.stringify(e.json, null, 2));
    }
  }

  /**
  *  Create Schema
  *  User Transaction
  */
  const createschema = async () => {

    for (const obj of data) {
      try {
        const result = await api.transact({
          actions: [{
            account: 'armyassets',
            name: 'createschema',
            authorization: [{
              actor: 'armyassets',
              permission: 'active',
            }],
            data: {
              authorized_creator: 'armyassets',
              collection_name: "aacollection",
              schema_name: obj?.SCHEMA,
              schema_format: [
                {
                  name:"TITLE",
                  type:"string",
                },
                {
                  name:"IMAGE",
                  type:"string",
                },
                {
                  name:"VALUE",
                  type:"int64",
                },
                {
                  name:"RARITY",
                  type:"int64",
                },              
              ],
            },
          }]
        }, {
          blocksBehind: 3,
          expireSeconds: 1200,
        });
        setCreateSchemaMessage(JSON.stringify(result));
        await new Promise(resolve => setTimeout(resolve, 1000));
        // await getCurrentMessage();
      } catch(e) {
        setCreateSchemaMessage(e.message);
        if (e instanceof RpcError)
          console.log(JSON.stringify(e.json, null, 2));
      }
    }
  }

  /**
  *  Create Template
  *  User Transaction
  */
  const createtemplate = async () => {

    for (const obj of data) {
      try {
        const result = await api.transact({
          actions: [{
            account: 'armyassets',
            name: 'createtempl',
            authorization: [{
              actor: 'armyassets',
              permission: 'active',
            }],
            data: {
              authorized_creator: 'armyassets',
              collection_name: "aacollection",
              schema_name: obj?.SCHEMA,
              transferable: obj?.TRANSFERABLE === "YES" ? true : false,
              burnable: obj?.BURNABLE === "YES" ? true : false,
              max_supply: obj?.SUPPLY,
              immutable_data: [
                {
                  key:"TITLE",
                  value:["string", obj?.TITLE],
                },
                {
                  key:"IMAGE",
                  value:["string", obj?.IMAGE],
                },
                {
                  key:"VALUE",
                  value:["int64", obj?.VALUE],
                },
                {
                  key:"RARITY",
                  value:["int64", obj?.RARITY],
                },
              ],
            },
          }]
        }, {
          blocksBehind: 3,
          expireSeconds: 1200,
        });
        setCreateTemplateMessage(JSON.stringify(result));
        await new Promise(resolve => setTimeout(resolve, 1000));
        // await getCurrentMessage();
      } catch(e) {
        setCreateTemplateMessage(e.message);
        if (e instanceof RpcError)
          console.log(JSON.stringify(e.json, null, 2));
      }
    }
  }

  /**
  *  Mint Assets
  *  User Transaction
  */
  const mintasset = async () => {

    try {
      const result = await api.transact({
        actions: [{
          account: 'armyassets',
          name: 'mintasset',
          authorization: [{
            actor: 'armyassets',
            permission: 'active',
          }],
          data: {
            authorized_minter: 'armyassets',
            collection_name: "aacollection",
            schema_name: "bb1111111111",
            template_id: 1,
            new_asset_owner: "armyassets",
            immutable_data: [
              {
                key:"TITLE",
                value:["string", "obj?.TITLE"],
              },
              {
                key:"IMAGE",
                value:["string", "obj?.IMAGE"],
              },
              {
                key:"VALUE",
                value:["int64", "obj?.VALUE"],
              },
              {
                key:"RARITY",
                value:["int64", "obj?.RARITY"],
              },
            ],
          },
        }]
      }, {
        blocksBehind: 3,
        expireSeconds: 1200,
      });
      setCreateTemplateMessage(JSON.stringify(result));
      await new Promise(resolve => setTimeout(resolve, 1000));
      // await getCurrentMessage();
    } catch(e) {
      setCreateTemplateMessage(e.message);
      if (e instanceof RpcError)
        console.log(JSON.stringify(e.json, null, 2));
    }
  }

  const handleFileUpload = e => {
    const file = e.target.files[0];
    const reader = new FileReader();
    reader.onload = (evt) => {
      /* Parse data */
      const bstr = evt.target.result;
      const wb = XLSX.read(bstr, { type: 'binary' });
      /* Get first worksheet */
      const wsname = wb.SheetNames[0];
      const ws = wb.Sheets[wsname];
      /* Convert array of arrays */
      const data = XLSX.utils.sheet_to_csv(ws, { header: 1 });
      processData(data);
    };
    reader.readAsBinaryString(file);
  }

  const processData = dataString => {
    const dataStringLines = dataString.split(/\r\n|\n/);
    const headers = dataStringLines[0].split(/,(?![^"]*"(?:(?:[^"]*"){2})*[^"]*$)/);
 
    const list = [];
    for (let i = 1; i < dataStringLines.length; i++) {
      const row = dataStringLines[i].split(/,(?![^"]*"(?:(?:[^"]*"){2})*[^"]*$)/);
      if (headers && row.length === headers.length) {
        const obj = {};
        for (let j = 0; j < headers.length; j++) {
          let d = row[j];
          if (d.length > 0) {
            if (d[0] === '"')
              d = d.substring(1, d.length - 1);
            if (d[d.length - 1] === '"')
              d = d.substring(d.length - 2, 1);
          }
          if (headers[j]) {
            obj[headers[j]] = d;
          }
        }
 
        // remove the blank rows
        if (Object.values(obj).filter(x => x).length > 0) {
          list.push(obj);
        }
      }
    }
 
    // prepare columns list from headers
    const columns = headers.map(c => ({
      name: c,
      selector: c,
    }));
 
    setData(list);
    setColumns(columns);
  }

  return (
    <div className="App">
      <header className="App-header">
          This is the test app for wax blockchain
      </header>
      <div className="App-content">
        <div className="App-admin">
          <h4 style={{color: 'white'}}>Admin Management</h4>
          <div className="App-button">
            <button onClick={init} className="sign-button">Init</button>
            <button onClick={admincoledit} className="sign-button">Admin Collection Edit</button>
          </div>
          <h5 style={{color: 'red', margin: 0}}>Init Message:</h5>
          <div>{initMessage}</div>
          <h5 style={{color: 'red', margin: 0}}>Admin Collection Edit Message:</h5>
          <div>{adminColEditMessage}</div>
        </div>
        <div className="App-user">
          <h4 style={{color: 'white'}}>User Transactions</h4>

          {/* Create Collection */}
          <div className="App-button">
            <button onClick={createcol} className="sign-button">Create Collection</button>
          </div>
          <h5 style={{color: 'red', margin: 0}}>Create Collection Message:</h5>
          <div>{createColMessage}</div>

          {/* CSV upload */}
          <h5 style={{color: 'white', padding: 0}}>NOTE: You should upload CSV file before creating Schema.</h5>
          <div className="csv-upload">
            <label htmlFor="csvUpload" style={{color:'white', fontSize:'1.8rem'}}>CSV File Upload : </label>
            <input 
                  type="file"
                  className="csv-upload-file"
                  id="csvUpload"
                  accept=".csv,.xlsx,.xls"
                  onChange={handleFileUpload}
              />
          </div>

          {/* Create Schema */}
          <div className="App-button">
            <button onClick={createschema} className="sign-button">Create Schema</button>
          </div>
          <h5 style={{color: 'red', margin: 0}}>Create Schema Message:</h5>
          <div>{createSchemaMessage}</div>

          {/* Create Template */}
          <div className="App-button">
            <button onClick={createtemplate} className="sign-button">Create Template</button>
          </div>
          <h5 style={{color: 'red', margin: 0}}>Create Template Message:</h5>
          <div>{createTemplateMessage}</div>
        </div>
          
      </div>
    </div>
  );
}

export default App;
