/* CDApplet.vapi generated by valac 0.18.1, do not modify. */

namespace CairoDock {
	namespace Applet {
		[CCode (cheader_filename = "CDApplet.h")]
		public class CDApplet : GLib.Object {
			public enum ContainerType {
				DOCK,
				DESKLET
			}
			public enum DialogKey {
				DIALOG_KEY_ENTER,
				DIALOG_KEY_ESCAPE
			}
			public enum EmblemModifier {
				EMBLEM_PERSISTENT,
				EMBLEM_PRINT
			}
			public enum EmblemPosition {
				EMBLEM_TOP_LEFT,
				EMBLEM_BOTTOM_RIGHT,
				EMBLEM_BOTTOM_LEFT,
				EMBLEM_TOP_RIGHT,
				EMBLEM_MIDDLE,
				EMBLEM_BOTTOM,
				EMBLEM_TOP,
				EMBLEM_RIGHT,
				EMBLEM_LEFT
			}
			public enum MenuItemId {
				MAIN_MENU_ID
			}
			public enum MenuItemType {
				MENU_ENTRY,
				MENU_SUB_MENU,
				MENU_SEPARATOR,
				MENU_CHECKBOX,
				MENU_RADIO_BUTTON
			}
			public enum ScreenPosition {
				BOTTOM,
				TOP,
				RIGHT,
				LEFT
			}
			public string cAppletName;
			public string cBusPath;
			public string cConfFile;
			public string cParentAppName;
			public string cRootDataDir;
			public string cShareDataDir;
			public CairoDock.Applet.IApplet icon;
			public CairoDock.Applet.ISubApplet sub_icons;
			public CDApplet (string[] argv);
			public virtual void begin ();
			public virtual void end ();
			public virtual void get_config (GLib.KeyFile keyfile);
			public virtual void on_answer (GLib.Variant answer);
			public virtual void on_answer_dialog (int iButton, GLib.Variant answer);
			public virtual void on_build_menu ();
			public virtual void on_build_menu_sub_icon (string cIconID);
			public virtual void on_change_focus (bool bIsActive);
			public virtual void on_click (int iState);
			public virtual void on_click_sub_icon (int iState, string cIconID);
			public virtual void on_drop_data (string cReceivedData);
			public virtual void on_drop_data_sub_icon (string cReceivedData, string cIconID);
			public virtual void on_menu_select (int iNumEntry);
			public virtual void on_menu_select_sub_icon (int iNumEntry, string cIconID);
			public virtual void on_middle_click ();
			public virtual void on_middle_click_sub_icon (string cIconID);
			public virtual void on_scroll (bool bScrollUp);
			public virtual void on_scroll_sub_icon (bool bScrollUp, string cIconID);
			public virtual void on_shortkey (string cKey);
			public virtual void reload ();
			public void run ();
		}
		[CCode (cheader_filename = "CDApplet.h")]
		[DBus (name = "org.cairodock.CairoDock.applet")]
		public interface IApplet : GLib.Object {
			public abstract void AddDataRenderer (string cType, int iNbValues, string cTheme) throws GLib.IOError;
			public abstract void AddMenuItems (GLib.HashTable<string,GLib.Variant>[] pItems) throws GLib.IOError;
			public abstract void Animate (string cAnimation, int iRounds) throws GLib.IOError;
			public abstract void BindShortkey (string[] cShortkeys) throws GLib.IOError;
			public abstract void ControlAppli (string cApplicationClass) throws GLib.IOError;
			public abstract void DemandsAttention (bool bStart, string cAnimation) throws GLib.IOError;
			public abstract GLib.Variant Get (string cProperty) throws GLib.IOError;
			public abstract GLib.HashTable<string,GLib.Variant> GetAll () throws GLib.IOError;
			public abstract void PopupDialog (GLib.HashTable<string,GLib.Variant> hDialogAttributes, GLib.HashTable<string,GLib.Variant> hWidgetAttributes) throws GLib.IOError;
			public abstract void RenderValues (double[] pValues) throws GLib.IOError;
			public abstract void SetEmblem (string cImage, int iPosition) throws GLib.IOError;
			public abstract void SetIcon (string cImage) throws GLib.IOError;
			public abstract void SetLabel (string cLabel) throws GLib.IOError;
			public abstract void SetQuickInfo (string cQuickInfo) throws GLib.IOError;
			public abstract void ShowAppli (bool bShow) throws GLib.IOError;
			public abstract void ShowDialog (string cMessage, int iDuration) throws GLib.IOError;
			public signal void on_answer (GLib.Variant answer);
			public signal void on_answer_dialog (int iButton, GLib.Variant answer);
			public signal void on_build_menu ();
			public signal void on_change_focus (bool bIsActive);
			public signal void on_click (int iState);
			public signal void on_drop_data (string cReceivedData);
			public signal void on_menu_select (int iNumEntry);
			public signal void on_middle_click ();
			public signal void on_reload_module (bool bConfigHasChanged);
			public signal void on_scroll (bool bScrollUp);
			public signal void on_shortkey (string cKey);
			public signal void on_stop_module ();
		}
		[CCode (cheader_filename = "CDApplet.h")]
		[DBus (name = "org.cairodock.CairoDock.subapplet")]
		public interface ISubApplet : GLib.Object {
			public abstract void AddSubIcons (string[] pIconFields) throws GLib.IOError;
			public abstract void Animate (string cAnimation, int iNbRounds, string cIconID) throws GLib.IOError;
			public abstract void RemoveSubIcon (string cIconID) throws GLib.IOError;
			public abstract void SetEmblem (string cImage, int iPosition, string cIconID) throws GLib.IOError;
			public abstract void SetIcon (string cImage, string cIconID) throws GLib.IOError;
			public abstract void SetLabel (string cLabel, string cIconID) throws GLib.IOError;
			public abstract void SetQuickInfo (string cQuickInfo, string cIconID) throws GLib.IOError;
			public abstract void ShowDialog (string message, int iDuration, string cIconID) throws GLib.IOError;
			public signal void on_build_menu_sub_icon (string cIconID);
			public signal void on_click_sub_icon (int iState, string cIconID);
			public signal void on_drop_data_sub_icon (string cReceivedData, string cIconID);
			public signal void on_middle_click_sub_icon (string cIconID);
			public signal void on_scroll_sub_icon (bool bScrollUp, string cIconID);
		}
	}
}
