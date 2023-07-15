#include "Bridge.h"

void lua_panic( sol::optional <std::string> message )
{
	if ( !message )
		return;

	auto log = "Lua error: " + message.value_or( "unknown" );
	Console->ColorPrint( log, Color(255, 0, 0) );
}
namespace client
{
	void print( const char* msg, Color clr )
	{
		Console->ColorPrint( msg, clr );
	}


}


void on_load_script()
{
	Lua->load_script( Lua->get_script_id( Config->lua_list->get( ) ) );
}

void script_unload()
{
	Lua->unload_script( Lua->get_script_id( Config->lua_list->get( ) ) );
}

void CLua::Setup()
{
	lua = sol::state( sol::c_call<decltype( &lua_panic ), &lua_panic> );
	sol::table client = lua.create_table();

	lua.new_usertype <Color>("color", sol::constructors <Color( ), Color( int, int, int ), Color( int, int, int, int )>( ),
		( std::string )"r", &Color::r,
		( std::string )"g", &Color::g,
		( std::string )"b", &Color::b,
		( std::string )"a", &Color::a
	);

	
	client[ "print" ] = client::print;

	lua["client"] = client;

	refresh_scripts();
	Config->lua_list->UpdateList(scripts_names);
	Config->lua_button->set_callback(on_load_script);
	Config->lua_button_unload->set_callback(script_unload);
}


int CLua::get_script_id( std::string name ) {
	for ( int i = 0; i < this->scripts.size( ); i++ ) {
		if ( this->scripts.at( i ) == name )
			return i;
	}
	return -1;
}

std::string CLua::get_script_path( std::string name ) {
	return this->get_script_path( this->get_script_id( name ) );
}

std::string CLua::get_script_path( int id ) {
	if ( id == -1 )
		return  "";

	return this->pathes.at( id ).string( );
}

void CLua::load_script( int id )
{
	if ( id == -1 )
		return;

	if ( loaded.at( id ) ) //-V106
		return;

	auto path = get_script_path( id );

	if ( path == "" )
		return;

	auto error_load = false;
	loaded.at( id ) = true;
	lua.script_file( path,
		[ &error_load ] ( lua_State*, sol::protected_function_result result )
		{
			if ( !result.valid( ) )
			{
				sol::error error = result;
				auto log = "Lua error: " + ( std::string )error.what( );
				error_load = true;
			}

			return result;
		}
	);

	if ( error_load | loaded.at( id ) == false )
	{
		loaded.at( id ) = false;
		return;
	}
	ctx.loaded_script = true;
}

void CLua::unload_script( int id ) {
	if ( id == -1 )
		return;

	if ( !loaded.at( id ) )
		return;

	if ( ctx.loaded_script )
		for ( auto current :Lua->hooks.getHooks("on_unload") )
			current.func( );

	ctx.loaded_script = false;

	hooks.unregisterHooks( id );
	loaded.at( id ) = false;
}

void CLua::refresh_scripts( )
{
	auto oldLoaded = loaded;
	auto oldScripts = scripts;

	loaded.clear( );
	pathes.clear( );
	scripts.clear( );
	scripts_names.clear( );

	std::vector<std::filesystem::path> pathes_to_scan = { "C:\\gs\\luas\\cloud","C:\\gs\\luas" };
	for ( int l = 0; l < 2; l++ ) {

		for ( auto& entry : std::filesystem::directory_iterator( pathes_to_scan.at( l ) ) )
		{
			if ( entry.path( ).extension( ) == ( ".lua" ) || entry.path( ).extension( ) == ( ".luac" ) )
			{
				auto path = entry.path( );
				auto filename = path.filename( ).string( );

				auto didPut = false;



				for ( auto i = 0; i < oldScripts.size( ); i++ )
				{
					if ( filename == oldScripts.at( i ) ) //-V106
					{
						loaded.push_back( oldLoaded.at( i ) ); //-V106
						didPut = true;
					}
				}

				if ( !didPut )
					loaded.push_back( false );

				pathes.push_back( path );
				scripts.push_back( filename );

				if ( l == 0 ) {
					scripts_names.push_back( "*" + filename );
				}
				else {
					scripts_names.push_back( filename );
				}
			}
		}
	}

}

CLua* Lua = new CLua;