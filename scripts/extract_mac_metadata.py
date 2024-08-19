import os
import os.path
import pickle


def extract_resource_fork_data(root: str) -> None:
    mac_md_root = os.path.join('.mac_md', root)

    for item in os.listdir(mac_md_root):
        mac_md_item_path = os.path.join(mac_md_root, item)
        item_path = os.path.join(root, item)
        if os.path.isdir(mac_md_item_path):
            extract_resource_fork_data(item_path)

        elif os.path.isfile(mac_md_item_path):
            assert os.path.basename(mac_md_item_path) == '.__mac_attr__', mac_md_item_path
            output_path = os.path.dirname(item_path) + '.rsrc'

            with open(mac_md_item_path, "rb") as f:
                attr_dict: dict[bytes, bytes] = pickle.load(f, encoding="bytes")

            if b'com.apple.ResourceFork' in attr_dict:
                rsf_data = attr_dict.pop(b'com.apple.ResourceFork')
                print(f'... {mac_md_item_path} => {output_path} ({len(rsf_data)} bytes)')
                os.makedirs(os.path.dirname(output_path), exist_ok=True)
                with open(output_path, "wb") as f:
                    f.write(rsf_data)


if __name__ == '__main__':
    extract_resource_fork_data('.')
