BEGIN_NS

namespace gmui
{
	struct GMUIConsole
	{
		static const GMWchar* UI;
	};
}

#define ID_EDIT_CONSOLE				_L("Edit_Console")
#define ID_TABLAYOUT				_L("TabLayout")
#define ID_OPTION_LOG				_L("Option_Log")
#define ID_OPTION_PERFORMACE		_L("Option_Performance")
#define	ID_OPTION_FILTER_INFO		_L("Option_Info")
#define	ID_OPTION_FILTER_WARNING	_L("Option_Warning")
#define	ID_OPTION_FILTER_ERROR		_L("Option_Error")
#define	ID_OPTION_FILTER_DEBUG		_L("Option_Debug")
#define	ID_PROFILE_GRAPH			_L("Option_Profile_Graph")

const GMWchar* gmui::GMUIConsole::UI = L""
	L"<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?> "
	L"<Window size=\"700,700\"> "
	L"    <VerticalLayout enabled=\"true\" bkcolor=\"#FFA6CAF0\"> "
	L"        <HorizontalLayout enabled=\"true\" width=\"700\" height=\"43\"> "
	L"            <Option name=\"" ID_OPTION_LOG L"\"text=\"Log\" float=\"true\" pos=\"8,7,0,0\" enabled=\"true\" width=\"60\" height=\"30\" textcolor=\"#FF000000\" disabledtextcolor=\"#FFA7A6AA\" align=\"center\" /> "
	L"            <Option name=\"" ID_OPTION_PERFORMACE L"\"text=\"Performance\" float=\"true\" pos=\"83,7,0,0\" enabled=\"true\" width=\"114\" height=\"30\" textcolor=\"#FF000000\" disabledtextcolor=\"#FFA7A6AA\" align=\"center\" /> "
	L"        </HorizontalLayout> "
	L"        <HorizontalLayout enabled=\"true\" width=\"700\"> "
	L"            <TabLayout name=\"" ID_TABLAYOUT L"\" enabled=\"true\" width=\"698\" height=\"500\"> "
	L"                <HorizontalLayout enabled=\"true\" width=\"698\" height=\"500\"> "
	L"                    <Option name=\"" ID_OPTION_FILTER_INFO L"\" text=\"Info\" float=\"true\" pos=\"8,7,0,0\" enabled=\"true\" width=\"60\" height=\"30\" />"
	L"                    <Option name=\"" ID_OPTION_FILTER_WARNING L"\" text=\"Warning\" float=\"true\" pos=\"8,47,0,0\" enabled=\"true\" width=\"60\" height=\"30\" />"
	L"                    <Option name=\"" ID_OPTION_FILTER_ERROR L"\" text=\"Error\" float=\"true\" pos=\"8,87,0,0\" enabled=\"true\" width=\"60\" height=\"30\" />"
	L"                    <Option name=\"" ID_OPTION_FILTER_DEBUG L"\" text=\"Debug\" float=\"true\" pos=\"8,127,0,0\" enabled=\"true\" width=\"60\" height=\"30\" />"
	L"                    <RichEdit name=\"" ID_EDIT_CONSOLE L"\" float=\"true\" pos=\"100,10,650,810\" enabled=\"true\" width=\"550\" height=\"500\" bkcolor=\"#FFFFFFFF\" autovscroll=\"true\" vscrollbar=\"true\"/> "
	L"                </HorizontalLayout> "
	L"                <HorizontalLayout enabled=\"true\" width=\"698\" height=\"500\" > "
	L"                    <GMGraph name=\"" ID_PROFILE_GRAPH L"\" pos=\"8,7,0,0\" width=\"200\" height=\"300\" />"
	L"                </HorizontalLayout>"
	L"            </TabLayout> "
	L"        </HorizontalLayout> "
	L"    </VerticalLayout> "
	L"</Window> "
;
END_NS