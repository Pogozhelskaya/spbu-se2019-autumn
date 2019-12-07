using System.Threading;

namespace Task05
{
    public class CoarseGrainedBinaryTree
    {
        public BinaryTreeNode Root;
        private readonly Mutex _mutex = new Mutex();
        public bool Find(int key)
        {
            _mutex.WaitOne();
            var result = FindNode(Root, key) != null;
            _mutex.ReleaseMutex();
            return result;
        }

        private BinaryTreeNode FindNode(BinaryTreeNode node, int key)
        {
            while (true)
            {
                if (node == null || node.Key == key)
                {
                    return node;
                }

                node = node.Key > key ? node.Left : node.Right;
            }
        }

        public void Insert(int key)
        {
            _mutex.WaitOne();
            InsertNode(new BinaryTreeNode(key));
        }

        private void InsertNode(BinaryTreeNode node)
        {
            if (Root == null)
            {
                Root = node;
                _mutex.ReleaseMutex();
            }
            else if (FindNode(Root, node.Key) == null)
            {
                var currentNode = Root;

                while (currentNode != null)
                {
                    if (node.Key > currentNode.Key)
                    {
                        if (currentNode.Right == null)
                        {
                            node.Parent = currentNode;
                            currentNode.Right = node;
                            _mutex.ReleaseMutex();
                            break;
                        }

                        currentNode = currentNode.Right;
                    }
                    else
                    {
                        if (currentNode.Left == null)
                        {
                            node.Parent = currentNode;
                            currentNode.Left = node;
                            _mutex.ReleaseMutex();
                            break;
                        }

                        currentNode = currentNode.Left;
                    }
                }
            }
            else
            {
                _mutex.ReleaseMutex();
            }
        }
        
    }
}